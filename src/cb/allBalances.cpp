// Dump balance of all addresses ever used in the blockchain

#include <util.h>
#include <common.h>
#include <errlog.h>
#include <option.h>
#include <rmd160.h>
#include <sha256.h>
#include <callback.h>

#include <vector>
#include <map>
#include <cmath>
#include <string.h>
#include <cstdio>
#include <sqlite3.h>

#define BALANCE_CLASSES 12

struct Addr;
static uint8_t emptyKey[kSHA256ByteSize] =
{ 0x52 };
typedef GoogMap<Hash160, Addr*, Hash160Hasher, Hash160Equal>::Map AddrMap;
typedef GoogMap<Hash160, int, Hash160Hasher, Hash160Equal>::Map RestrictMap;

struct Output
{
	int64_t time;
	int64_t value;
	uint64_t inputIndex;
	uint64_t outputIndex;
	const uint8_t *upTXHash;
	const uint8_t *downTXHash;
};

typedef std::vector<Output> OutputVec;

typedef std::map<unsigned long, unsigned> BalanceMap;

struct Addr
{
	uint64_t sum;
	uint64_t nbIn;
	uint64_t nbOut;
	uint160_t hash;
	uint32_t lastIn;
	uint32_t lastOut;
	OutputVec *outputVec;
};

template<> uint8_t *PagedAllocator<Addr>::pool = 0;
template<> uint8_t *PagedAllocator<Addr>::poolEnd = 0;
static inline Addr *allocAddr()
{
	return (Addr*) PagedAllocator<Addr>::alloc();
}

struct CompareAddr
{
	bool operator()(const Addr * const &a, const Addr * const &b) const
	{
		return (b->sum) < (a->sum);
	}
};

struct AllBalances: public Callback
{
	bool detailed;
	int64_t limit;
	uint64_t offset;
	int64_t showAddr;
	int64_t cutoffBlock;
	optparse::OptionParser parser;

	AddrMap addrMap;
	uint32_t blockTime;
	const Block *curBlock;
	const Block *lastBlock;
	const Block *firstBlock;
	RestrictMap restrictMap;
	std::vector<Addr*> allAddrs;
	std::vector<uint160_t> restricts;

	int64_t addrNum;
	int64_t nonZeroCnt;
	struct tm timeStruct;
	unsigned char lastMonth = 0;
	unsigned short lastYear = 0;
	char timeBuf[256];
	FILE* outfile;

	unsigned long idBalance=1, idLogBalance=1, idDate=0;


	AllBalances()
	{
		parser.usage("[options] [list of addresses to restrict output to]").version(
				"").description(
				"dump the balance for all addresses that appear in the blockchain").epilog(
				"");
		parser.add_option("-a", "--atBlock").action("store").type("int").set_default(
				-1).help(
				"only take into account transactions in blocks strictly older than <block> (default: all)");
		parser.add_option("-l", "--limit").action("store").type("int").set_default(
				-1).help(
				"limit output to top N balances, (default : output all addresses)");
		parser.add_option("-w", "--withAddr").action("store").type("int").set_default(
				500).help(
				"only show address for top N results (default: N=%default)");
		parser.add_option("-d", "--detailed").action("store_true").set_default(
				false).help("also show all unspent outputs");
		parser.add_option("-o", "--outputFile").action("store").set_default(
				"out.txt").help("");
	}

	virtual const char *name() const
	{
		return "allBalances";
	}
	virtual const optparse::OptionParser *optionParser() const
	{
		return &parser;
	}
	virtual bool needTXHash() const
	{
		return true;
	}

	virtual void aliases(std::vector<const char*> &v) const
	{
		v.push_back("balances");
	}

	virtual int init(int argc, const char *argv[])
	{
		offset = 0;
		curBlock = 0;
		lastBlock = 0;
		firstBlock = 0;

		addrMap.setEmptyKey(emptyKey);
		addrMap.resize(15 * 1000 * 1000);
		allAddrs.reserve(15 * 1000 * 1000);

		optparse::Values &values = parser.parse_args(argc, argv);
		cutoffBlock = values.get("atBlock");
		showAddr = values.get("withAddr");
		detailed = values.get("detailed");
		limit = values.get("limit");
		const char* filename = values.get("outputFile");
		outfile = fopen(filename, "w");
		if (outfile == NULL)
		{
			printf("Impossible to open file: \'%s\'\n", filename);
			exit(-1);
		}

		const char* init = "BEGIN; CREATE TABLE date(id INTEGER PRIMARY KEY, timestamp INTEGER, block_count INTEGER);\n"
		"CREATE TABLE LogBalanceSegment(id INTEGER PRIMARY KEY,logBalance INTEGER, count INTEGER, date_id INTEGER);\n"
		"CREATE TABLE BalanceSegment(id INTEGER PRIMARY KEY,balanceClass INTEGER, count INTEGER, date_id INTEGER);\n\n";
		fprintf(outfile, init);

		auto args = parser.args();
		for (size_t i = 1; i < args.size(); ++i)
		{
			loadKeyList(restricts, args[i].c_str());
		}

		if (0 <= cutoffBlock)
		{
			info(
					"only taking into account transactions before block %" PRIu64 "\n",
					cutoffBlock);
		}

		if (0 != restricts.size())
		{

			info("restricting output to %" PRIu64 " addresses ...\n",
					(uint64_t) restricts.size());

			auto e = restricts.end();
			auto i = restricts.begin();
			restrictMap.setEmptyKey(emptyKey);
			while (e != i)
			{
				const uint160_t &h = *(i++);
				restrictMap[h.v] = 1;
			}
		} else
		{
			if (detailed)
			{
				warning(
						"asking for --detailed for *all* addresses in the blockchain will be *very* slow");
				warning(
						"as a matter of fact, it likely won't ever finish unless you have *lots* of RAM");
			}
		}

		return 0;
	}

	void move(const uint8_t *script, uint64_t scriptSize,
			const uint8_t *upTXHash, int64_t outputIndex, int64_t value,
			const uint8_t *downTXHash = 0, uint64_t inputIndex = -1)
	{
		uint8_t addrType[3];
		uint160_t pubKeyHash;
		int type = solveOutputScript(pubKeyHash.v, script, scriptSize,
				addrType);
		if (unlikely(type < 0))
			return;

		if (0 != restrictMap.size())
		{
			auto r = restrictMap.find(pubKeyHash.v);
			if (restrictMap.end() == r)
			{
				return;
			}
		}

		Addr *addr;
		auto i = addrMap.find(pubKeyHash.v);
		if (unlikely(addrMap.end() != i))
		{
			addr = i->second;
		} else
		{

			addr = allocAddr();

			memcpy(addr->hash.v, pubKeyHash.v, kRIPEMD160ByteSize);
			addr->outputVec = 0;
			addr->nbOut = 0;
			addr->nbIn = 0;
			addr->sum = 0;

			if (detailed)
			{
				addr->outputVec = new OutputVec;
			}

			addrMap[addr->hash.v] = addr;
			allAddrs.push_back(addr);
		}

		if (0 < value)
		{
			addr->lastIn = blockTime;
			++(addr->nbIn);
		} else
		{
			addr->lastOut = blockTime;
			++(addr->nbOut);
		}
		addr->sum += value;

		if (detailed)
		{
			struct Output output;
			output.value = value;
			output.time = blockTime;
			output.upTXHash = upTXHash;
			output.downTXHash = downTXHash;
			output.inputIndex = inputIndex;
			output.outputIndex = outputIndex;
			addr->outputVec->push_back(output);
		}
	}

	virtual void endOutput(const uint8_t *p, uint64_t value,
			const uint8_t *txHash, uint64_t outputIndex,
			const uint8_t *outputScript, uint64_t outputScriptSize)
	{
		move(outputScript, outputScriptSize, txHash, outputIndex, value);
	}

	static void gmTime(struct tm &timeStruct, char *timeBuf, const time_t &last)
	{
		gmtime_r(&last, &timeStruct);
		asctime_r(&timeStruct, timeBuf);

		size_t sz = strlen(timeBuf);
		if (0 < sz)
			timeBuf[sz - 1] = 0;
	}

	virtual void edge(uint64_t value, const uint8_t *upTXHash,
			uint64_t outputIndex, const uint8_t *outputScript,
			uint64_t outputScriptSize, const uint8_t *downTXHash,
			uint64_t inputIndex, const uint8_t *inputScript,
			uint64_t inputScriptSize)
	{
		move(outputScript, outputScriptSize, upTXHash, outputIndex,
				-(int64_t) value, downTXHash, inputIndex);
	}


	unsigned computeClass(uint64_t balance)
	{
		if(balance==0)
		{
			return 0;
		}
		else if(balance == 1)
		{
			return 1;
		}
		else if(balance < (uint64_t)(0.001*100e6))
		{
			return 2;
		}
		else if(balance < (uint64_t)(0.01*100e6))
		{
			return 3;
		}
		else if(balance < (uint64_t)(0.1*100e6))
		{
			return 4;
		}
		else if(balance < (uint64_t)(100e6))
		{
			return 5;
		}
		else if(balance < (uint64_t)(10*100e6))
		{
			return 6;
		}
		else if(balance < (uint64_t)(100*100e6))
		{
			return 7;
		}
		else if(balance < (uint64_t)(1000*100e6))
		{
			return 8;
		}
		else if(balance < (uint64_t)(10000*100e6))
		{
			return 9;
		}
		else if(balance < (uint64_t)(100000*100e6))
		{
			return 10;
		}
		else
		{
			return 11;
		}
	}

	void dumpStats()
	{

		CompareAddr compare;
		auto e = allAddrs.end();
		auto s = allAddrs.begin();
		std::sort(s, e, compare);

		uint64_t nbRestricts = (uint64_t) restrictMap.size();

		addrNum = 0;
		nonZeroCnt = 0;
		BalanceMap balanceMap;
		unsigned balanceArray[BALANCE_CLASSES];
		for(unsigned &u : balanceArray)
		{
			u=0;
		}

		while (likely(s < e))
		{

			if (0 <= limit && limit <= addrNum)
				break;

			Addr *addr = *(s++);
			if (0 != nbRestricts)
			{
				auto r = restrictMap.find(addr->hash.v);
				if (restrictMap.end() == r)
					continue;
			}

			if (0 < addr->sum)
			{
				++nonZeroCnt;
			}

			int exp = addr->sum > 0 ? log10(addr->sum) * 16 + 1 : 0;
			if (balanceMap.find(exp) == balanceMap.end())
			{
				balanceMap[exp] = 1;
			}
			else
			{
				balanceMap[exp] += 1;
			}
			balanceArray[computeClass(addr->sum)]++;

			++addrNum;
		}

		std::string insert;
		for (auto x : balanceMap)
		{
			insert = "INSERT INTO LogBalanceSegment(id, logBalance, count, date_id) VALUES(%lu, %lu, %u, %lu);\n";
			fprintf(outfile, insert.c_str(), idLogBalance++, x.first, x.second, idDate);
		}

		for(unsigned i=0; i<BALANCE_CLASSES; i++)
		{
			insert = "INSERT INTO BalanceSegment(id, balanceClass, count, date_id) VALUES(%lu, %lu, %u, %lu);\n";
			fprintf(outfile, insert.c_str(), idBalance++, i, balanceArray[i], idDate);
		}


		fprintf(outfile, "\n");
	}

	virtual void start(const Block *s, const Block *e)
	{
		firstBlock = s;
		lastBlock = e;
	}

	virtual void startLC()
	{
		info("computing balance for all addresses");
	}

	virtual void startBlock(const Block *b, uint64_t chainSize)
	{
		curBlock = b;

		const uint8_t *p = b->chunk->getData();
		const uint8_t *sz = -4 + p;
		LOAD(uint32_t, size, sz);
		offset += size;

		double now = usecs();
		static double startTime = 0;
		static double lastStatTime = 0;
		double elapsed = now - lastStatTime;
		bool longEnough = (5 * 1000 * 1000 < elapsed);
		bool closeEnough = ((chainSize - offset) < 80);
		if (unlikely(longEnough || closeEnough))
		{

			if (0 == startTime)
			{
				startTime = now;
			}

			double progress = offset / (double) chainSize;
			double elasedSinceStart = 1e-6 * (now - startTime);
			double speed = progress / elasedSinceStart;
			info("%8" PRIu64 " blocks, "
			"%8.3f MegaAddrs , "
			"%6.2f%% , "
			"elapsed = %5.2fs , "
			"eta = %5.2fs , ", curBlock->height, addrMap.size() * 1e-6,
					100.0 * progress, elasedSinceStart,
					(1.0 / speed) - elasedSinceStart);

			lastStatTime = now;
		}

		SKIP(uint32_t, version, p);
		SKIP(uint256_t, prevBlkHash, p);
		SKIP(uint256_t, blkMerkleRoot, p);
		LOAD(uint32_t, bTime, p);
		blockTime = bTime;

		gmTime(timeStruct, timeBuf, blockTime);

		if (timeStruct.tm_mon > lastMonth || timeStruct.tm_year > lastYear)
		{
			std::string insert_date = "INSERT INTO DATE(id, timestamp, block_count) VALUES(%lu, %lu, %lu);\n";
			fprintf(outfile, insert_date.c_str(), ++idDate, mktime(&timeStruct), b->height);

			std::cout << timeBuf << std::endl;
			//timeVector.push_back(std::string(timeBuf));
			lastYear = timeStruct.tm_year;
			lastMonth = timeStruct.tm_mon;
			dumpStats();
		}

		if (0 <= cutoffBlock && cutoffBlock <= curBlock->height)
		{
			wrapup();
		}
	}

	virtual void wrapup()
	{
		info("done\n");
		info("found %" PRIu64 " addresses with non zero balance",
				nonZeroCnt);
		info("found %" PRIu64 " addresses in total",
				(uint64_t) allAddrs.size());
		info("shown:%" PRIu64 " addresses", (uint64_t) addrNum);
		fprintf(outfile, "COMMIT;\n");

		std::string view = "CREATE VIEW mainViewLog AS SELECT trunc(1e-8*10^((b.logbalance-1)/16.0), 3) as min,"
				" trunc(1e-8*10^(b.logbalance/16.0),3) as max, b.count, datetime(d.timestamp, 'unixepoch') as mese,"
				" d.block_count from LogBalanceSegment b, date d where b.date_id=d.id order by d.timestamp;\n";
		fprintf(outfile, view.c_str());

		view = "CREATE VIEW mainViewLog AS SELECT trunc(1e-8*10^((b.logbalance-1)/16.0), 3) as min,"
				" trunc(1e-8*10^(b.logbalance/16.0),3) as max, b.count, to_timestamp(d.timestamp) as mese,"
				" d.block_count from LogBalanceSegment b, date d where b.date_id=d.id order by d.timestamp;\n";
		fprintf(outfile, view.c_str());

		fprintf(outfile, "%s\n", "CREATE VIEW mainView AS SELECT b.balanceClass, b.count, datetime(d.timestamp, 'unixepoch') "
						"as mese, d.block_count from BalanceSegment b, date d where b.date_id=d.id order by d.timestamp;");
		fprintf(outfile, "%s\n", "CREATE VIEW mainView AS SELECT b.balanceClass, b.count, to_timestamp(d.timestamp) "
						"as mese, d.block_count from BalanceSegment b, date d where b.date_id=d.id order by d.timestamp;");
		//fclose(outfile);
		exit(0);
	}

	~AllBalances()
	{
		fclose(outfile);
	}

};

static AllBalances allBalances;
