/// \file util.hpp
#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include <fwdpp/forward_types.hpp>
#include <fwdpp/fwd_functional.hpp>
#include <set>
#include <map>
#include <algorithm>
#include <cassert>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <iostream>

#ifndef USE_STANDARD_CONTAINERS
#include <boost/container/vector.hpp>
#endif


namespace KTfwd
{
  template< typename gamete_type,
	    typename vector_allocator_type,
	    typename mutation_type,
	    typename list_allocator_type,
	    template<typename,typename> class vector_type,
	    template<typename,typename> class list_type >
  void valid_copy( const vector_type<gamete_type,vector_allocator_type> & gametes,
		   const list_type<mutation_type,list_allocator_type> & mutations,
		   vector_type<gamete_type,vector_allocator_type> & gametes_destination,
		   list_type<mutation_type,list_allocator_type> & mutations_destination )
  /*!
    If you ever need to store (and later restore) the state of the population, a naive copy operation
    is not sufficient, because of all the pointers from the gametes container to elements
    of the mutations container.  Use this function instead.

    \note Only works for the case of unique mutation positions!!!
  */
  {
    BOOST_STATIC_ASSERT( (boost::is_same<typename gamete_type::mutation_type,mutation_type>::value) );

    typedef typename list_type<mutation_type,list_allocator_type>::iterator literator;  
    typedef typename list_type<mutation_type,list_allocator_type>::const_iterator cliterator;
    typedef typename gamete_type::mutation_container::const_iterator gciterator;
    gametes_destination.clear();
    mutations_destination.clear();
    //copying the mutations is trivial
    std::map<double,literator> mutlookup;
    for( cliterator i = mutations.begin();
	 i!=mutations.end();++i)
      {
	literator j = mutations_destination.insert(mutations_destination.end(),*i);
	mutlookup[j->pos]=j;
      }
    
    for(unsigned i=0;i<gametes.size();++i)
      {
	//copy construct so that all public, etc., data
	//are properly initialized
	gamete_type new_gamete(gametes[i]);
	new_gamete.mutations.clear();
	new_gamete.smutations.clear();
	for(gciterator itr = gametes[i].mutations.begin() ; 
	    itr != gametes[i].mutations.end() ; ++itr)
	  {
	    new_gamete.mutations.push_back( mutlookup[(*itr)->pos] );
	  }
	for(gciterator itr = gametes[i].smutations.begin() ; 
	    itr != gametes[i].smutations.end() ; ++itr)
	  {
	    new_gamete.smutations.push_back( mutlookup[(*itr)->pos] );
	  }
	gametes_destination.push_back(new_gamete);
      }
  }

  template<typename mutation_type,
	   typename list_type_allocator,
	   template <typename,typename> class list_type>
  void uncheck( list_type<mutation_type,list_type_allocator> * mutations )
  /*! \brief Used internally
   */
  {
    BOOST_STATIC_ASSERT( ( boost::is_base_and_derived<mutation_base,mutation_type>::value) );
    typedef typename list_type<mutation_type,list_type_allocator>::iterator mlitr;
    for(mlitr i = mutations->begin();i!=mutations->end();++i)
      {
	i->checked=false;
      }
  }
  
  /*! \brief Remove mutations from population
    Removes mutations that are lost.
    \example diploid.cc
  */
  template<typename mutationtype,
	   typename list_type_allocator,
	   template <typename,typename> class list_type >
  void remove_lost( list_type<mutationtype,list_type_allocator> * mutations )
  {
    BOOST_STATIC_ASSERT( ( boost::is_base_and_derived<mutation_base,mutationtype>::value) );
    typename list_type<mutationtype,list_type_allocator>::iterator i = mutations->begin();
    
    while(i != mutations->end())
      {
	i->checked = false;
	if( i->n == 0 )
	  {
	    mutations->erase(i);
	    i=mutations->begin();
	  }
	else
	  {
	    ++i;
	  }
      }
  }

 /*! \brief Remove mutations from population
    Removes mutations that are lost.
    \note: lookup must be compatible with lookup->erase(lookup->find(double))
    \example diploid.cc
   */
  template<typename mutationtype,
	   typename list_type_allocator,
	   template <typename,typename> class list_type,
	   typename mutation_lookup_table>
  void remove_lost( list_type<mutationtype,list_type_allocator> * mutations, mutation_lookup_table * lookup )
  {
    BOOST_STATIC_ASSERT( ( boost::is_base_and_derived<mutation_base,mutationtype>::value) );
    typename list_type<mutationtype,list_type_allocator>::iterator i = mutations->begin();
    
    while(i != mutations->end())
      {
	i->checked = false;
	if( i->n == 0 )
	  {
	    lookup->erase(lookup->find(i->pos));
	    mutations->erase(i);
	    i=mutations->begin();
	  }
	else
	  {
	    ++i;
	  }
      }
  }

  /*! \brief Remove mutations from population
    Removes mutations that are fixed or lost.
    \example diploid.cc
   */
  template<typename mutationtype,
	   typename vector_type_allocator1,
	   typename vector_type_allocator2,
	   typename list_type_allocator,
	   template <typename,typename> class vector_type,
	   template <typename,typename> class list_type >
  void remove_fixed_lost( list_type<mutationtype,list_type_allocator> * mutations, 
			  vector_type<mutationtype,vector_type_allocator1> * fixations, 
			  vector_type<unsigned,vector_type_allocator2> * fixation_times,
			  const unsigned & generation,const unsigned & twoN)
	   {
	     BOOST_STATIC_ASSERT( ( boost::is_base_and_derived<mutation_base,mutationtype>::value) );
	     typename list_type<mutationtype,list_type_allocator>::iterator i = mutations->begin();
	     
	     while(i != mutations->end())
	       {
		 assert(i->n <= twoN);			
		 i->checked = false;
		 if(i->n==twoN )
		   {
		     fixations->push_back(*i);
		     fixation_times->push_back(generation);
		   }
		 if( i->n == 0 || i->n == twoN )
		   {
		     mutations->erase(i);
		     i=mutations->begin();
		   }
		 else
		   {
		     ++i;
		   }
	       }
	   }

  /*! \brief Remove mutations from population
    Removes mutations that are fixed or lost.
    \note: lookup must be compatible with lookup->erase(lookup->find(double))
    \example diploid.cc
   */
  template<typename mutationtype,
	   typename vector_type_allocator1,
	   typename vector_type_allocator2,
	   typename list_type_allocator,
	   template <typename,typename> class vector_type,
	   template <typename,typename> class list_type,
	   typename mutation_lookup_table>
  void remove_fixed_lost( list_type<mutationtype,list_type_allocator> * mutations, 
			  vector_type<mutationtype,vector_type_allocator1> * fixations, 
			  vector_type<unsigned,vector_type_allocator2> * fixation_times,
			  mutation_lookup_table * lookup,
			  const unsigned & generation,const unsigned & twoN )
  {
    BOOST_STATIC_ASSERT( ( boost::is_base_and_derived<mutation_base,mutationtype>::value) );
    typename list_type<mutationtype,list_type_allocator>::iterator i = mutations->begin();
    
    while(i != mutations->end())
      {
	assert(i->n <= twoN);			
	i->checked = false;
	if(i->n==twoN )
	  {
	    fixations->push_back(*i);
	    fixation_times->push_back(generation);
	  }
	if( i->n == 0 || i->n == twoN )
	  {
	    lookup->erase(lookup->find(i->pos));
	    mutations->erase(i);
	    i=mutations->begin();
	  }
	else
	  {
	    ++i;
	  }
      }
  }
  
  //template<typename gamete_type>
  template<typename iterator_type>
  void adjust_mutation_counts( iterator_type g , const unsigned & n)
  /*! \brief used internally
    \note Will need a specialization if types have other data that need updating
  */
  {
    for(unsigned j=0;j< g->mutations.size();++j)
      {
	if( g->mutations[j]->checked==false)
	  {
	    g->mutations[j]->n=n;
	    g->mutations[j]->checked=true;
	  }
	else
	  {
	    g->mutations[j]->n += n;
	  }
      }
    for(unsigned j=0;j< g->smutations.size();++j)
      {
	if( g->smutations[j]->checked==false)
	  {
	    g->smutations[j]->n=n;
	    g->smutations[j]->checked=true;
	  }
	else
	  {
	    g->smutations[j]->n += n;
	  }
      }
  }

  /*! \brief Pick a gamete proportional to its frequency
    Pick a gamete proportional to its frequency
   */
  template< typename gamete_type,
	    typename vector_type_allocator,
	    template <typename,typename> class vector_type > 
  typename vector_type<gamete_type,vector_type_allocator>::iterator
  pgam( gsl_rng * r,
	vector_type<gamete_type,vector_type_allocator > * gametes )
  {
    typedef typename vector_type<gamete_type,vector_type_allocator>::iterator vcitr;
    vcitr rv = gametes->begin();
#ifndef USE_STANDARD_CONTAINERS
    boost::container::vector<double>freqs(gametes->size(),0);
#else
    std::vector<double>freqs(gametes->size(),0);
#endif
    size_t i = 0;
    for( ; rv != gametes->end() ; ++rv,++i )
      {
	freqs[i]=rv->n;
      }
    gsl_ran_discrete_t * lookup = gsl_ran_discrete_preproc(gametes->size(),&freqs[0]);
    rv = gametes->begin()+gsl_ran_discrete(r,lookup);
    gsl_ran_discrete_free(lookup);
    return rv;
  }
										  
  template< typename gamete_type,
	    typename vector_type_allocator,
	    typename mutation_removal_policy,
	    template <typename,typename> class vector_type >
  void update_gamete_list( vector_type<gamete_type,vector_type_allocator > * gametes, 
			   const unsigned & twoN,
			   const mutation_removal_policy & mrp)
  {
    gametes->erase(std::remove_if(gametes->begin(),
     				  gametes->end(),
     				  boost::bind(n_is_zero(),_1)),
     		   gametes->end()); 
    for(typename vector_type<gamete_type,vector_type_allocator >::iterator gbeg = gametes->begin() ; 
	gbeg != gametes->end() ; ++gbeg )
      {
	assert(gbeg->n <= twoN);
	gbeg->mutations.erase( std::remove_if( gbeg->mutations.begin(),
					       gbeg->mutations.end(),
					       mrp ),
			       gbeg->mutations.end() );
	gbeg->smutations.erase( std::remove_if( gbeg->smutations.begin(),
						gbeg->smutations.end(),
						mrp ),
				gbeg->smutations.end() );
      }

  }

  /*! \brief Multinomial sampling of gametes
    Called by KTfwd::sample_diploid
    \param r GSL random number generator
    \param gbegin Iterator to the start of a vector of gametes
    \param ngametes The size of the vector whose beginning is gbegin
    \param efreqs Vector of frequencies from which to sample.  Must sum to 1 (not checked!).  Typically, this would be the expected frequency of gbegin to (gbegin+i) in the next generation
    \param twoN Twice the number of diploids
   */
  template<typename iterator_type>
  void multinomial_sample_gametes(gsl_rng * r,iterator_type gbegin, 
  				  const size_t & ngametes, 
  				  const double * efreqs, 
  				  const unsigned & twoN)
  {
    unsigned n = twoN;
    double sum=1.;
    for(unsigned i=0;i<ngametes;++i)
      {
    	if( (*(efreqs+i)) > 0. )
    	  {
    	    (gbegin+i)->n = gsl_ran_binomial(r,*(efreqs+i)/sum,n);
    	    sum -= *(efreqs+i);
    	  }
    	else
    	  {
    	    (gbegin+i)->n = 0;
    	  }
    	n -= (gbegin+i)->n;
    	//adjust_mutation_counts( &*(gbegin+i), (gbegin+i)->n );
	adjust_mutation_counts( (gbegin+i), (gbegin+i)->n );
      }
  }
}
#endif /* _UTIL_HPP_ */