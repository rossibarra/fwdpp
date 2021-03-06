#ifndef __FWDPP_EXPERIMENTAL_SAMPLE_DIPLOID_HPP__
#define __FWDPP_EXPERIMENTAL_SAMPLE_DIPLOID_HPP__

#include <fwdpp/diploid.hh>

namespace KTfwd {
  namespace experimental {
    /*!
      \brief Abstraction of the standard Wright-Fisher sampling process
     */
    struct standardWFrules
    {
      mutable double wbar;
      mutable std::vector<double> fitnesses;

      mutable fwdpp_internal::gsl_ran_discrete_t_ptr lookup;
      //! \brief Constructor
      standardWFrules() : wbar(0.),fitnesses(std::vector<double>()),lookup(fwdpp_internal::gsl_ran_discrete_t_ptr(nullptr))
      {
      }

      //! \brief The "fitness manager"
      template<typename dipcont_t,
	       typename gcont_t,
	       typename mcont_t,
	       typename fitness_func>
      void w(const dipcont_t & diploids,
	     gcont_t & gametes,
	     const mcont_t & mutations,
	     const fitness_func & ff)const
      {
	using diploid_geno_t = typename dipcont_t::value_type;
	unsigned N_curr = diploids.size();
	if(fitnesses.size() < N_curr) fitnesses.resize(N_curr);
	wbar = 0.;

	for(unsigned i=0;i<N_curr;++i)
	  {
	    gametes[diploids[i].first].n=gametes[diploids[i].second].n=0;
	    fitnesses[i]= fwdpp_internal::diploid_fitness_dispatch(ff,diploids[i],gametes,mutations,
								   typename KTfwd::traits::is_custom_diploid_t<diploid_geno_t>::type());
	  }
	
	wbar /= double(diploids.size());

	/*!
	  Black magic alert:
	  fwdpp_internal::gsl_ran_discrete_t_ptr contains a std::unique_ptr wrapping the GSL pointer.
	  This type has its own deleter, which is convenient, because
	  operator= for unique_ptrs automagically calls the deleter before assignment!
	  Details: http://www.cplusplus.com/reference/memory/unique_ptr/operator=
	*/
	lookup = fwdpp_internal::gsl_ran_discrete_t_ptr(gsl_ran_discrete_preproc(N_curr,&fitnesses[0]));
      }

      //! \brief Pick parent one
      inline size_t pick1(gsl_rng * r) const
      {
	return gsl_ran_discrete(r,lookup.get());
      }

      //! \brief Pick parent 2.  Parent 1's data are passed along for models where that is relevant
      template<typename diploid_t,typename gcont_t,typename mcont_t>
      inline size_t pick2(gsl_rng * r, const size_t & p1, const double & f,
			  const diploid_t &, const gcont_t &, const mcont_t &) const
      {
	return ((f==1.)||(f>0.&&gsl_rng_uniform(r) < f)) ? p1 : gsl_ran_discrete(r,lookup.get());
      }

      //! \brief Update some property of the offspring based on properties of the parents
      template<typename diploid_t,typename gcont_t,typename mcont_t>
      void update(gsl_rng * , diploid_t &, const diploid_t & ,
		  const diploid_t &,
		  const gcont_t &,
		  const mcont_t &) const
      {
      }

    };

    //single deme, N changing

    //! \brief Experimental variant where the population rules are implemented via an external policy
    template< typename gamete_type,
	      typename gamete_list_type_allocator,
	      typename mutation_type,
	      typename mutation_list_type_allocator,
	      typename diploid_geno_t,
	      typename diploid_vector_type_allocator,
	      typename diploid_fitness_function,
	      typename mutation_model,
	      typename recombination_policy,
	      template<typename,typename> class gamete_list_type,
	      template<typename,typename> class mutation_list_type,
	      template<typename,typename> class diploid_vector_type,
	      typename popmodel_rules = standardWFrules,
	      typename mutation_removal_policy = std::true_type,
	      typename gamete_insertion_policy = KTfwd::emplace_back
	      >
    double
    sample_diploid(gsl_rng * r,
		   gamete_list_type<gamete_type,gamete_list_type_allocator > & gametes,
		   diploid_vector_type<diploid_geno_t,diploid_vector_type_allocator> & diploids,
		   mutation_list_type<mutation_type,mutation_list_type_allocator > & mutations,
		   std::vector<uint_t> & mcounts,
		   const unsigned & N_curr, 
		   const unsigned & N_next, 
		   const double & mu,
		   const mutation_model & mmodel,
		   const recombination_policy & rec_pol,
		   const diploid_fitness_function & ff,
		   typename gamete_type::mutation_container & neutral,
		   typename gamete_type::mutation_container & selected,
		   const double & f = 0.,
		   const popmodel_rules & pmr = popmodel_rules(),
		   const mutation_removal_policy & mp = mutation_removal_policy(),
		   const gamete_insertion_policy & gpolicy_mut = gamete_insertion_policy())
    {
      assert(N_curr == diploids.size());

      auto gamete_recycling_bin = fwdpp_internal::make_gamete_queue(gametes);
      auto mutation_recycling_bin = fwdpp_internal::make_mut_queue(mcounts);
      auto gamete_lookup = fwdpp_internal::gamete_lookup_table(gametes,mutations);
      pmr.w(diploids,gametes,mutations,ff);

#ifndef NDEBUG
      for(const auto & g : gametes) assert(!g.n);
#endif
      const auto parents(diploids); //copy the parents
    
      //Change the population size
      if( diploids.size() != N_next)
	{
	  diploids.resize(N_next);
	}
      for(auto & dip : diploids )
	{
	  //Pick parent 1
	  size_t p1 = pmr.pick1(r);
	  //Pick parent 2
	  size_t p2 = pmr.pick2(r,p1,f,parents[p1],gametes,mutations);
	  assert(p1<parents.size());
	  assert(p2<parents.size());
	
	  std::size_t p1g1 = parents[p1].first;
	  std::size_t p1g2 = parents[p1].second;
	  std::size_t p2g1 = parents[p2].first;
	  std::size_t p2g2 = parents[p2].second;

	  //0.3.3 change:
	  if(gsl_rng_uniform(r)<0.5) std::swap(p1g1,p1g2);
	  if(gsl_rng_uniform(r)<0.5) std::swap(p2g1,p2g2);
	  
	  dip.first = recombination(gametes,gamete_lookup,gamete_recycling_bin,
				    neutral,selected,rec_pol,p1g1,p1g2,mutations).first;
	  dip.second = recombination(gametes,gamete_lookup,gamete_recycling_bin,
				     neutral,selected,rec_pol,p2g1,p2g2,mutations).first;
	
	  gametes[dip.first].n++;
	  gametes[dip.second].n++;

	  //now, add new mutations
	  dip.first = mutate_gamete_recycle(mutation_recycling_bin,gamete_recycling_bin,r,mu,gametes,mutations,dip.first,mmodel,gpolicy_mut);
	  dip.second = mutate_gamete_recycle(mutation_recycling_bin,gamete_recycling_bin,r,mu,gametes,mutations,dip.second,mmodel,gpolicy_mut);
	  
	  assert( gametes[dip.first].n );
	  assert( gametes[dip.second].n );
	  pmr.update(r,dip,parents[p1],parents[p2],gametes,mutations);
	}
#ifndef NDEBUG
      for(const auto & dip : diploids)
	{
	  assert(gametes[dip.first].n);
	  assert(gametes[dip.first].n<=2*N_next);
	  assert(gametes[dip.second].n);
	  assert(gametes[dip.second].n<=2*N_next);
	}
#endif
      fwdpp_internal::process_gametes(gametes,mutations,mcounts);
      assert(popdata_sane(diploids,gametes,mcounts));
      fwdpp_internal::gamete_cleaner(gametes,mcounts,2*N_next,typename std::is_same<decltype(mp),KTfwd::remove_nothing >::type());
      assert(check_sum(gametes,2*N_next));
      return pmr.wbar;
    }

    //single deme, N constant

    //! \brief Experimental variant where the population rules are implemented via an external policy
     template< typename gamete_type,
	      typename gamete_list_type_allocator,
	      typename mutation_type,
	      typename mutation_list_type_allocator,
	      typename diploid_geno_t,
	      typename diploid_vector_type_allocator,
	      typename diploid_fitness_function,
	      typename mutation_model,
	      typename recombination_policy,
	      template<typename,typename> class gamete_list_type,
	      template<typename,typename> class mutation_list_type,
	      template<typename,typename> class diploid_vector_type,
	      typename popmodel_rules = standardWFrules,
	      typename mutation_removal_policy = std::true_type,
	      typename gamete_insertion_policy = KTfwd::emplace_back
	      >
    double
    sample_diploid(gsl_rng * r,
		   gamete_list_type<gamete_type,gamete_list_type_allocator > & gametes,
		   diploid_vector_type<diploid_geno_t,diploid_vector_type_allocator> & diploids,
		   mutation_list_type<mutation_type,mutation_list_type_allocator > & mutations,
		   std::vector<uint_t> & mcounts,
		   const unsigned & N_curr, 
		   const double & mu,
		   const mutation_model & mmodel,
		   const recombination_policy & rec_pol,
		   const diploid_fitness_function & ff,
		   typename gamete_type::mutation_container & neutral,
		   typename gamete_type::mutation_container & selected,
		   const double & f = 0.,
		   const popmodel_rules & pmr = popmodel_rules(),
		   const mutation_removal_policy & mp = mutation_removal_policy(),
		   const gamete_insertion_policy & gpolicy_mut = gamete_insertion_policy())
    {
      return experimental::sample_diploid(r,gametes,diploids,mutations,mcounts,N_curr,N_curr,mu,mmodel,rec_pol,ff,neutral,selected,f,pmr,mp,gpolicy_mut);
    }
  }
}
#endif
