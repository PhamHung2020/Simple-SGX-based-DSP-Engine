## Dataset

### Nexmark

- [X] Q1: 245
- [X] Q2
    - [X] Filter: 304
    - [X] Map: 249
- [X] Q3
    - [X] Filter Auction: 372
    - [X] Filter Person: 300
    - [x] Join: 1008
    - [X] Map Join Result: 384
- [X] Q4
    - [X] Join: 1010
    - [X] Map Join Result: 384
    - [X] Max: 512
    - [X] Join Category: 512
    - [X] Average: 1018
- [X] Q5
    - [X] Count: 750
    - [X] Max: 750
- [X] Q6
    - [X] Join = Q4
    - [X] Filter: 321
    - [X] Max: 1094
    - [X] Avg: 603

Total: 9816
- Map: 1262
- Filter: 1297
- Join (2 streams): 2018
- Join (1 stream): 512
- Count+GroupBy: 750
- Max (without GroupBy): 750
- Max+GroupBy: 1606
- Average (sliding window): 1018
- Average (partition window): 1071


### SecureStreams (~1K)

Paper: [SecureStream](https://arxiv.org/pdf/1805.01752)

- [X] Dataset
- [X] Implementation
- Queries:
    - [X] Map: 350
    - [X] Filter: 350
    - [X] Reduce: 350

Total: 1050

### StreamBox (~2K): 2 weeks (10 - 23/2)

Paper: [StreamBox-TZ](https://www.usenix.org/conference/atc19/presentation/park-heejin)

Queries
-  [ ] Top Value Per Key
    -  [ ] Dataset
    -  [ ] Implementation
-  [ ] Counting Unique Taxis*
    -  [X] Dataset
    -  [ ] Implementation
-  [ ] Temporal Join*
    -  [ ] Dataset
    -  [ ] Implementation
-  [ ] Window Sum*
    -  [X] Dataset
    -  [ ] Implementation
-  [ ] Filter (+Window???)*
    -  [ ] Dataset
    -  [ ] Implementation
-  [ ] Power Grid (Max)
    -  [ ] Dataset
    -  [ ] Implementation

### Flink (~2K):???

Article: [Uber's AthenaX](https://www.uber.com/en-AU/blog/athenax/)

- [ ] Dataset
- [ ] Implementation
- [ ] Queries