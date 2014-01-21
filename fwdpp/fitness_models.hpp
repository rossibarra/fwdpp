#ifndef _FITNESS_MODELS_HPP_
#define _FITNESS_MODELS_HPP_

#include <fwdpp/forward_types.hpp>
#include <fwdpp/fwd_functional.hpp>
#include <fwdpp/fitness_policies.hpp>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>

namespace KTfwd
{
  /*! \brief Returns a fitness of 1
    \return A fitness of 1
    \param g1 A gamete
    \param g2 A gamete

    \note g1 and g2 must be part of the gamete_base hierarchy
  */
  struct no_selection
  {
    typedef double result_type;
    template<typename iterator_type >
    inline result_type operator()(const iterator_type & g1, const iterator_type &g2) const
    {
      BOOST_STATIC_ASSERT( (boost::is_base_and_derived<mutation_base,typename iterator_type::value_type::mutation_type>::value) );
      return 1.;
    }
  };

  /*! \brief Function object for fitness as a function of individual mutations in a diploid

    Function object for fitness as a function of mutations in a diploid.  Examples include the standard multiplicative and additive models of population genetics.  This routine idenfifies all homozygous/heterozygous mutations in a diploid and updates the diploid's fitness according to user-defined policies.  See the code for KTfwd::multiplicative_diploid and KTfwd::additive_diploid for specific examples.
    \param g1 An iterator to a gamete
    \param g2 An iterator to a gamete
    \param fpol_hom Policy for updating fitness for the case of homozygosity for a mutant
    \param fpol_het Policy for updating fitness for the case of heterozygosity for a mutant
    \param starting_fitness The value to which the function will initialize the return value
    \return The fitness of a diploid with genotype g1 and g2
    \note The updating policies must take a non-const reference to a double as the first argument and
    an iterator to a gamete as the second.  Any remaining arguments needed should be passed via a mechanism such as boost::bind.  See KTfwd::multiplicative_fitness_updater_hom and KTfwd::multiplicative_fitness_updater_het for examples.  The iterators g1 and g2 must point to objects in the class hierarchy of KTfwd::gamete_base.

    \note This function is unwieldy to call directly.  It is best to define your two policies and write a wrapper function calling this function. See the code for KTfwd::multiplicative_diploid and KTfwd::additive_diploid for specific examples.  
   */
  struct site_dependent_fitness
  {
    typedef double result_type;
    template<typename iterator_type,
	     typename fitness_updating_policy_hom,
	     typename fitness_updating_policy_het>
    inline result_type operator()( const iterator_type & g1,
				   const iterator_type & g2,
				   const fitness_updating_policy_hom & fpol_hom,
				   const fitness_updating_policy_het & fpol_het,
				   const double starting_fitness = 1. ) const
    {
      BOOST_STATIC_ASSERT( (boost::is_base_and_derived<mutation_base,typename iterator_type::value_type::mutation_type>::value) );
      result_type fitness=starting_fitness;
      if( g1->smutations.empty() && g2->smutations.empty() ) return fitness;
      typename iterator_type::value_type::mutation_list_type_iterator ib1,ib2;
      typename iterator_type::value_type::mutation_container::const_iterator b1=g1->smutations.begin(),
	e1=g1->smutations.end(),
	b2=g2->smutations.begin(),
	e2=g2->smutations.end();
      //This is a fast way to calculate fitnesses,
      //as it just compares addresses in memory, and 
      //does little in the way of dereferencing and storing
      bool found = false;
      for( ; b1 < e1 ; ++b1 )
	{
	  found = false;
	  ib1 = *b1;
	  for( ; !found && b2 < e2 && !((*b2)->pos > (ib1)->pos) ; ++b2 )
	    {
	      ib2 = *b2;
	      if ( ib2 == ib1 ) //homozygote
		{
		  assert(ib1->s == ib2->s); //just making sure
		  assert(ib1->pos == ib2->pos);
		  fpol_hom(fitness,ib1);
		  found=true;
		}
	      else 
		//b2 points to a unique mutation that comes before b1
		{
		  assert(ib2->pos != ib1->pos);
		  assert(ib2->pos < ib1->pos);
		  fpol_het(fitness,ib2);
		}
	    }
	  if(!found) //het
	    {
	      fpol_het(fitness,ib1);
	    }
	}
      for( ; b2 < e2 ; ++b2)
	{	
	  ib2=*b2;
	  fpol_het(fitness,ib2);
	}
      return fitness;
    }
  };

  /*! \brief Function object for fitness as a function of the two haplotypes in a diploid

    \param g1 An iterator to a gamete
    \param g2 An iterator to a gamete
    \param hpol A policy whose first argument is an iterator to a gamete. Remaining arguments may be bound via boost::bind or the equivalent.  The policy returns a double representing the effect of this haplotype on fitness
    \param dpol A policy whose first two arguments are doubles, each of which represents the effect of g1 and g2, respectively.  Remaining arguments may be bound via boost::bind or the equivalent.  The policy returns a double representing the fitness of a diploid genotype g1/g2
    \return dpol( hpol(g1), hpol(g2) )
    \note This really is just a convenience function. Depending on the specifics of the model, this function may be totally unnecessary.
  */
  struct haplotype_dependent_fitness
  {
    typedef double result_type;
    template< typename iterator_type,
	      typename haplotype_policy,
	      typename diploid_policy >
    inline result_type operator()(const iterator_type & g1,
				  const iterator_type & g2,
				  const haplotype_policy & hpol,
				  const diploid_policy & dpol) const
    {
      BOOST_STATIC_ASSERT( (boost::is_base_and_derived<mutation_base,typename iterator_type::value_type::mutation_type>::value) );
      return dpol( hpol(g1), hpol(g2) );
    }
  };

  /*! \brief Multiplicative fitness across sites
    \param g1 An iterator pointing to a gamete
    \param g2 An iterator pointing to a gamete
    \param scaling Fitnesses are 1, 1+h*s, 1+scaling*s, for AA,Aa,aa, resp.  This parameter lets you make sure your
    simulation is on the same scale as various formula in the literature
    \return Multiplicative fitness across sites = site_dependent_fitness()(g1,g2,
    boost::bind(multiplicative_fitness_updater_hom(),_1,_2,scaling),
    boost::bind(multiplicative_fitness_updater_het(),_1,_2),
    1.);
  */
  struct multiplicative_diploid
  {
    typedef double result_type;
    template< typename iterator_type>
    inline double operator()(const iterator_type & g1, const iterator_type & g2, 
			     const double scaling = 1.) const
    {
      return site_dependent_fitness()(g1,g2,
				      boost::bind(multiplicative_fitness_updater_hom(),_1,_2,scaling),
				      boost::bind(multiplicative_fitness_updater_het(),_1,_2),
				      1.);
    }
  };

  /*! \brief Additive fitness across sites
    \param g1 An iterator pointing to a gamete
    \param g2 An iterator pointing to a gamete
    \param scaling Fitnesses are 1, 1+h*s, 1+scaling*s, for AA,Aa,aa, resp.  This parameter lets you make sure your
    simulation is on the same scale as various formula in the literature
    \return Additive fitness across sites: 1. + KTfwd::site_dependent_fitness()(g1,g2,
    boost::bind(KTfwd::additive_fitness_updater_hom(),_1,_2,scaling),
    boost::bind(KTfwd::additive_fitness_updater_het(),_1,_2),
    0.);
    \note g1 and g2 must be part of the gamete_base hierarchy
  */
  struct additive_diploid
  {
    typedef double result_type;
    template< typename iterator_type>
    inline double operator()(const iterator_type & g1, const iterator_type & g2, 
			     const double scaling = 1.) const
    {
      return 1. + site_dependent_fitness()(g1,g2,
					   boost::bind(additive_fitness_updater_hom(),_1,_2,scaling),
					   boost::bind(additive_fitness_updater_het(),_1,_2),
					   0.);
    }
  };
}
#endif /* _FITNESS_MODELS_HPP_ */