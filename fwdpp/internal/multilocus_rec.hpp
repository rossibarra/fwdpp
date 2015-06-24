#ifndef __FWDPP_INTERNAL_MULTILOCUS_REC_HPP__
#define __FWDPP_INTERNAL_MULTILOCUS_REC_HPP__

/*
  The mechanics of crossing over for a multilocus
  simulation
 */

#include <gsl/gsl_rng.h>

namespace KTfwd {
  namespace fwdpp_internal {

    //0.3.3 test version
    /*
      Should take the following as args:
      1. the w/in locus rec policies
      2. the b/w locus rec policies
      3. non-const reference to offspring
      4. non-const copy of parent (temporary)

      It should:
      1. pre-compute all w/in locus breaks
      2. pre-compute all b/w locus crossover #s
      3. pre-count # of swaps everybody needs.

      return:
      void?
     */
    template<typename diploid_type,
	     typename recombination_policy_container,
	     typename bw_locus_rec_fxn
	     >
    void multilocus_rec(gsl_rng * r,
			diploid_type parent, //Copy--this is intentional
			diploid_type & offspring, //non-const ref, again intentional
			const recombination_policy_container & rec_pols,
			const bw_locus_rec_fxn & blrf,
			double * r_bw_loci
			)
    {
      //I see the problem: how to get the positions ahead of time...
    }
    
    /*! \brief The mechanics of recombination for multilocus simulations
      The mechanics of recombination for multilocus simulations
      \param r A GSL random number generator
      \param rec A policy that affects recombination withing a locus
      \param bw A policy that returns the number of recombination events between loci.  The return value must be unsigned int, and what really matters is if that value is odd or even.
      \param r_between_loci A const array of doubles corresponding to the recombination rates between loci.  For k loci, this vector must be k-1 doubles long.  Further, the i-th value must correspond to the recombination rate between loci i-1 and i.
      \param i A dummy index set equal to which locus we are processing.  Start counting from 0.
      \param parental_gamete_1 An iterator to the first parental gamete
      \param parental_gamete_2 An iterator to the second parental gamete
      \param g1 If true, then parental_gamete_1 is inherited by the descendant.
      \param LO Should be true if the last recombination even within loci resulted in an odd number of crossovers.
     */
    template<typename within_loc_rec_policy,
	     typename between_loc_rec_policy,
	     typename gamete_itr_t,
	     typename glookup_t>
    gamete_itr_t multilocus_rec(gsl_rng * r,
				const within_loc_rec_policy & rec,
				const between_loc_rec_policy & bw,
				const double * r_between_loci,
				const unsigned & i,
				gamete_itr_t & parental_gamete_1,
				gamete_itr_t & parental_gamete_2,
				glookup_t & gamete_lookup,
				bool & g1, bool & LO,
				bool & swapped )
    {
      /*
	This is the within-locus recombination policy.  It must conform
	to any single-locus policy.
       */
      if(!i)
	{
	  unsigned temp = rec(parental_gamete_1,parental_gamete_2,gamete_lookup);
	  LO = (temp%2!=0.);
	}
      else
	{
	  unsigned nrbw = bw(r,r_between_loci[i-1]);
	  bool obw=(nrbw%2!=0.);
	  if(obw&&!LO) {
	    std::swap(parental_gamete_1,parental_gamete_2);
	    swapped = true;
	  } else swapped=false;
	  unsigned temp = rec(parental_gamete_1,parental_gamete_2,gamete_lookup);
	  LO = (temp%2!=0.);
	}
      //IDEA: last # of xovers was "odd" in total.
      // if ( i > 0 )
      // 	{
      // 	  // std::cerr << "g1 and LO = " << g1 << ' ' << LO << ' ' << swapped << '\n';
      // 	  unsigned nrbw = bw(r,r_between_loci[i-1]);
      // 	  //std::cerr << "nrbw =" << nrbw << ' ';
      // 	  bool obw = (nrbw%2!=0) ? true : false;
      // 	  g1 = (LO) ? !g1 : g1;
      // 	  g1 = (obw) ? !g1 : g1;
      // 	  std::cerr <<i << ' '<< g1 << '\n';
      // 	  //WARNING. IDEA: These swaps are wrong for > 2 loci.
      // 	  if(!g1&&!swapped) {
      // 	    std::swap(parental_gamete_1,parental_gamete_2);
      // 	    swapped = true;
      // 	  } else swapped = false;
      // 	}
      // else
      // 	{
      // 	  std::cerr<< i << ' ' <<g1<<'\n';
      // 	}
      // //std::cerr << ", g1 = " << g1 << '\n';
      // unsigned temp = rec( parental_gamete_1,parental_gamete_2,gamete_lookup );
      // LO = (temp % 2 != 0.) ? true : false;
      // //return (g1) ? parental_gamete_1 : parental_gamete_2;
      // //IDEA:
      // std::cerr << "leaving: " << i << ' ' << g1 << ' ' << LO << ' ' << swapped << '\n';
      return parental_gamete_1;
    }
  }
}

#endif
