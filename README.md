## Doux: Decoupling Values from Keys for Real-Time Analytics

Doux is built upon the open-source code of Bourbon. [Bourbon](https://www.usenix.org/conference/osdi20/presentation/dai) is a fine-tuned version of Wisckey using learned indexes. Bourbon's source code is [here](https://bitbucket.org/daiyifandanny/learned-leveldb/src/master/). Since Bourbon already implements Wisckey, we extend it by integrating our method (Doux) along with other comparable approaches, such as DiffKV.

For detailed instructions on reproducing our experiments, please check out [ARTIFACT_README](./ARTIFACT_README) for step-by-step guidance on reimplementation.