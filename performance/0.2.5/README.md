#Using C++11 in fwdpp

Veresion 0.2.5 of fwdpp is the beginning of a migration from "classic" C++ to the C++11 language standard.   In practice, that means that many very useful features that have only been available via the [boost](http://www.boost.org) libraries are now available in the C++ standard library.

This directory represents a set of tests that I performed on the UCI cluster to check that "de-boosting" fwdpp won't adversely affect performance.

##What I am not showing you

I only show mean run time and mean peak memory usage below.  For all tests, the 0.2.5 and the 0.2.4 versions of the code give identical results for the same random number seed/parameter combox.  All tests were based on 128 replicates of each theta/rho combo, and the seeds used were 1 through 128 for all parameter sets.

##Some notes

1. Relative performance differences between 0.2.4 and 0.2.5 are likely highly parameter-dependent.  The fwdpp paper shows that the performance various simulation engines depends a lot on how much selection, etc., is going on. 
2. These results also apply to the author's system(s).
3. The tests from the 0.2.4/published/dev branch are re-run for each test, in order that each test be comparable.  I attempt to have 32 processes from version 0.2.4 and 32 from 0.2.5 running with the same parameters on the same node at the same time.  That doesn't work out perfectly in practice, but it is helpful when there is the possibility of the node itself affecting mean performance.

##Test1: std::bind vs. boost::bind

This test is of the following conditions:

1. The dev branch of fwdpp (which is the same as the published version 0.2.4 as far as this is concerned) was compiled using GCC 4.7.3 (using -std=c++11 -O2 -g for CXXFLAGS) on the UCI HPC, which is a cluster of AMD0-based 64-core nodes.  The example programs were compiled against libsequence 1.8.0 and boost 1.54.0.  In all cases, boost containers are used by the example program (see below) to store mutations and gametes, because it is faster.
2. The cpp11 branch of fwdpp where the only difference from the above is that boost::bind was replace with std::bind
3. The example program diploid_fixed_sh_ind was used to simulate a population of N=10,000 diploids for 8N generations with 4Nu = 4Nr and 4Nd = 5, which is twice the number of deleterious mutations entering the population per generation).  The script to run the simulation is:

```{sh}
#!/bin/bash

cd $SGE_O_WORKDIR

module load krthornt/libsequence/1.8.0
module load boost/1.54.0

if [ ! -s time.$SGE_TASK_ID.txt ]
then
    /usr/bin/time -f "%e %M" -o time.$SGE_TASK_ID.txt $1/diploid_fixed_sh_ind 10000 $2 5 $3 -0.001 0.5 80000 50 1 $SGE_TASK_ID > out.$SGE_TASK_ID.txt
fi
```

The run times and memory usage are basically the same using std::bind vs boost::bind, with an arguably advantage to std::bind.  In the figures, "0.2.4" refers to the dev branch/published version of fwdpp, and "0.2.5" will refer to the C++11 branch.

Run times:
![test1time](t1_time.png?raw=true "Run time and bind")

Memory (you only see one line because there is no difference between the two versions, so they overlap completely):
![test1mem](t1_mem.png?raw=true "Memory and bind")

##A fuller C++11 implementation of fwdpp

For this test, the cpp11 branch looks like:

1.  boost::bind was swapped for std::bind, and boost::function for std::function.  These two changes represent getting rid of all boost features in fwdpp's internal code except the use of boost containers in strategic places.
2.  The recombination routine for individual-based simulations was modified to only search the gamete list once.  This gives a noticeable performance boost, with some caveats.  The caveats are that I only see the performance difference on our AMD chips when compiling with GCC 4.8 or newer, and did not see it with 4.7.3.  However, on my development machine (Intel Xeon), I get a big boost with GCC 4.7 and with clang++ 3.4.  There appears to be a compiler by platform interaction, but I cannot rule out that it has more to do with how GCC 4.7.3 was configured on the cluster.

In addition:

1.  All programs were compiled with GCC 4.8.2, using -std=c++11 -O2 -g for CXXFLAGS.

We see a clear speedup due to the modified recombination routine:
![test2time](t2_time.png?raw=true "Run times for test2")

Again, memory use is unaffected:

![test2mem](t2_mem.png?raw=true "Memory for test2")

##Test 3: changing some book-keeping

I profiled the example program diploid_fixed_sh_ind for large theta/rho and discovered that adjust_mutation_counts was being called a lot, and I became suspicious that I was calling it too often.  This function is a book-keeper that makes sure that the mutation frequencies are properly updated after gamets are sampled.  In fwdpp 0.2.4 (and all of the above tests), it was being called twice per diploid per generation.  I moved it so that it is only called E(G) times per generation on average, where E(G) is the expected number of gametes in the population during the simulation.  This does not affect the output at all, but it does affect the speed.  For this test, this change is only applied to the cpp11 branch.

![test3time](t3_time.png?raw-true "Run-times for test3")

Relative performance of v. 0.2.5 vs 0.2.4:

![test3perf](t3_perf.png?raw-true "Relative performance increase for test3")

##Test 4: more profiling

I profiled the 0.2.5 code base resulting from the previous test and found that a lot of time was being spent in the function remove_fixed_lost.  It turns out that that function was going over the mutation vector many times per generation out of a combination of fear of iterator invalidation and an apparent unwillingness to code it up more efficiently.  Anhow, that is fixed now, and the various sample_diploid functions also have one less pass through the gamete list per generation.  The former change (improving remove_fixed_lost) has a huge effect on speed.

Run times:

![test4time](t4_time.png?raw=true "Run-times for test4")

Relative performance of v. 0.2.5 vs 0.2.4:

![test4perf](t4_perf.png?raw-true "Relative performance increase for test4")

##Test 5: lots of changes.

In this test, I compare the same 0.2.5 code as in Test 4 to what I will call "0.2.5_test", which is a version of the code with loads of changes:

1.  Lots of use of lambda expressions.
2.  Replacing for loops with std::for_each plus lambda expressions
3.  Using a mix of std::find_if and std::upper_bound during mutation and recombination
4.  Removing the sort step during the recombination.  (This was in 0.2.4 out of paranoia, but it is not necessary).

These changes give some performance improvements for larger regions:

![test5time](t5_time.png?raw=true "Run times for test5")

And we see the relative perfomance vs 0.2.4 improve (0.2.5_test is the red line):

![test5perf](t5_perf.png?raw=true "Relative performance for test5")

###Test 6: Small-N, large theta/rho

In the fwdpp paper, there was a clear scaling problem for large regions.  Let's revisit that with the following command, simulation N=1000 for 8N generations, and rho=theta = {1e3,2.5e3,5e3}, corresponding to 1MB,2.5MB, and 5MB regions the human genome.

```{sh}
/usr/bin/time -f "%e %M" -o time.$SGE_TASK_ID.txt $1/diploid_fixed_sh_ind 1000 $2 5 $3 -0.001 0.5 8000 50 1 $SGE_TASK_\
ID > out.$SGE_TASK_ID.txt 
```

![test6time](t6_time.png?raw=true "Run times for test 5")

The cpp11_test branch is close to 10x faster faster for the 5MB region:

![test6perf](t6_perf.png?raw=true "Relative performance for test 6")