#ifndef _EA_INDIVIDUAL_H_
#define _EA_INDIVIDUAL_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <ea/meta_data.h>

namespace ea {
	
	/*! Definition of an individual.
	 
	 Individuals within ealib are three things:
	 1) A container for a representation
	 2) A container for a fitness
	 3) A container for meta data
	 
	 By supporting arbitrary meta data, individuals can have location, lineage, or
	 other information attached to them.  We require that they at least contain a
	 representation and a fitness value.
	 */
	template <typename Representation, typename FitnessFunction>
	class individual {
	public:
		typedef Representation representation_type;
		typedef FitnessFunction fitness_function_type;
		typedef typename fitness_function_type::fitness_type fitness_type;
        typedef individual<representation_type,fitness_function_type> individual_type;
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
		
		//! Constructor.
		individual() : _name(0), _generation(0.0), _update(0) {
		}
        
		//! Constructor that builds an individual from a representation.
		individual(const representation_type& r) : _name(0), _generation(0.0), _update(0), _repr(r) {
		}
        
        //! Copy constructor.
        individual(const individual& that) {
            _name = that._name;
            _generation = that._generation;
            _update = that._update;
            _fitness = that._fitness;
            _repr = that._repr;
            _md = that._md;
        }
        
        //! Assignment operator.
        individual& operator=(const individual& that) {
            if(this != &that) {
                _name = that._name;
                _generation = that._generation;
                _update = that._update;
                _fitness = that._fitness;
                _repr = that._repr;
                _md = that._md;
            }
            return *this;
        }
        
        //! Destructor.
        virtual ~individual() {
        }
        
        //! Retrieve this individual's name.
        long& name() { return _name; }
		
        //! Retrieve this individual's id.
        const long& name() const { return _name; }

        //! Retrieve this individual's generation.
        double& generation() { return _generation; }

        //! Retrieve this individual's generation.
        const double& generation() const { return _generation; }
        
        //! Retrieve this individual's update.
        long& update() { return _update; }

        //! Retrieve this individual's update.
        const long& update() const { return _update; }

        //! Cast this individual to its fitness.
		operator fitness_type() { return _fitness; }

		//! Retrieve this individual's fitness.
		fitness_type& fitness() { return _fitness; }
		
		//! Retrieve this individual's fitness (const-qualified).
		const fitness_type& fitness() const { return _fitness; }
		
		//! Retrieve this individual's representation.
		representation_type& repr() { return _repr; }
        
		//! Retrieve this individual's representation.
		const representation_type& repr() const { return _repr; }
        
        //! Retrieve this individual's meta data.
        meta_data& md() { return _md; }

        //! Retrieve this individual's meta data.
        const meta_data& md() const { return _md; }

	protected:
        long _name; //!< Name (id number) of this individual.
        double _generation; //!< Generation of this individual.
        long _update; //!< Update at which this individual was born.
		fitness_type _fitness; //!< This individual's fitness.
		representation_type _repr; //!< This individual's representation.
        meta_data _md; //!< This individual's meta data.
        
	private:
		/* These enable serialization and de-serialization of individuals.
		 This would be easy, except for the fact that we can't round-trip NaNs.  So,
		 we store a flag instead of fitness in that case.
         */ 
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            ar & boost::serialization::make_nvp("name", _name);
            ar & boost::serialization::make_nvp("generation", _generation);
			bool null_fitness=_fitness.is_null();
			ar & BOOST_SERIALIZATION_NVP(null_fitness);
			if(!null_fitness) {
				ar & boost::serialization::make_nvp("fitness", _fitness);
			}
			ar & boost::serialization::make_nvp("representation", _repr);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("update", _update);
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("name", _name);
            ar & boost::serialization::make_nvp("generation", _generation);
			bool null_fitness=true;
			ar & BOOST_SERIALIZATION_NVP(null_fitness);
			if(null_fitness) {
                _fitness.nullify();
			} else {
				ar & boost::serialization::make_nvp("fitness", _fitness);
			}
			ar & boost::serialization::make_nvp("representation", _repr);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("update", _update);
		}
        
		BOOST_SERIALIZATION_SPLIT_MEMBER();
	};	

	/*! Compare individuals by their fitness.
	 */
	template <typename Representation, typename FitnessFunction>
	bool operator<(const individual<Representation,FitnessFunction>& a, const individual<Representation,FitnessFunction>& b) {
		return a.fitness() < b.fitness();
	}
    
    
    /*! Serialize an individual.
     */
    template <typename EA>
    void individual_save(std::ostream& out, typename EA::individual_type& ind, EA& ea) {
        boost::archive::xml_oarchive oa(out);
        oa << boost::serialization::make_nvp("individual",ind);
    }
    
    
	/*! Load a previously serialized individual.
     */
	template <typename EA>
    typename EA::individual_type individual_load(std::istream& in, EA& ea) {
        typename EA::individual_type ind;
		boost::archive::xml_iarchive ia(in);
        ia >> boost::serialization::make_nvp("individual", ind);
        return ind;
	}
	
    template <typename EA>
    typename EA::individual_type individual_load(const std::string& fname, EA& ea) {
        std::ifstream ifs(fname.c_str());
        return individual_load(ifs, ea);
    }

} // ea

#endif