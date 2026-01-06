## Doux: Decoupling Values from Keys for Real-Time Analytics

Doux is built upon the open-source code of Bourbon. [Bourbon](https://www.usenix.org/conference/osdi20/presentation/dai) is a fine-tuned version of Wisckey using learned indexes. Bourbon's source code is [here](https://bitbucket.org/daiyifandanny/learned-leveldb/src/master/). Since Bourbon already implements Wisckey, we extend it by integrating our method (Doux) along with other comparable approaches, such as DiffKV.

For detailed instructions on reproducing our experiments, please check out [ARTIFACT_README](./ARTIFACT_README) for step-by-step guidance on reimplementation.


## Data Generate

TPCH data for experiments is generated using the bundled `tpch-tool.tar.gz`:

1. Extract the tool:
   - `tar -xzf tpch-tool.tar.gz`
2. Follow the instructions in `tpch-tool/README.md` to build `dbgen` and generate tables with the desired scale factor.
3. A simple example (generate 1GB-scale `lineitem` and rename the file):
   - `cd tpch-tool/TPCH/dbgen && ./dbgen -T L -s 1 -f && mv -f lineitem.tbl lineitem.1`

For more options (different tables, scale factors, etc.), please refer to the detailed documentation in `tpch-tool/README.md`.