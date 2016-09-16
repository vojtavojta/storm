#ifndef STORM_BUILDER_EXPLICITMODELBUILDER_H
#define	STORM_BUILDER_EXPLICITMODELBUILDER_H

#include <memory>
#include <utility>
#include <vector>
#include <deque>
#include <cstdint>
#include <boost/functional/hash.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/variant.hpp>
#include "src/models/sparse/StandardRewardModel.h"

#include "src/storage/prism/Program.h"
#include "src/storage/expressions/ExpressionEvaluator.h"
#include "src/storage/BitVectorHashMap.h"
#include "src/logic/Formulas.h"
#include "src/models/sparse/StateAnnotation.h"
#include "src/models/sparse/Model.h"
#include "src/models/sparse/StateLabeling.h"
#include "src/storage/SparseMatrix.h"
#include "src/storage/sparse/StateValuations.h"
#include "src/storage/sparse/StateStorage.h"
#include "src/settings/SettingsManager.h"

#include "src/utility/prism.h"

#include "src/builder/ExplorationOrder.h"

#include "src/generator/NextStateGenerator.h"
#include "src/generator/CompressedState.h"
#include "src/generator/VariableInformation.h"

namespace storm {
    namespace utility {
        template<typename ValueType> class ConstantsComparator;
    }
    
    namespace builder {
        
        using namespace storm::utility::prism;
        using namespace storm::generator;
        
        // Forward-declare classes.
        template <typename ValueType> struct RewardModelBuilder;
        
        template<typename ValueType, typename RewardModelType = storm::models::sparse::StandardRewardModel<ValueType>, typename StateType = uint32_t>
        class ExplicitModelBuilder {
        public:
            // A structure holding the individual components of a model.
            struct ModelComponents {
                ModelComponents();
                
                // The transition matrix.
                storm::storage::SparseMatrix<ValueType> transitionMatrix;
                
                // The state labeling.
                storm::models::sparse::StateLabeling stateLabeling;
                
                // The reward models associated with the model.
                std::unordered_map<std::string, storm::models::sparse::StandardRewardModel<typename RewardModelType::ValueType>> rewardModels;
                
                // A vector that stores a labeling for each choice.
                boost::optional<std::vector<boost::container::flat_set<uint_fast64_t>>> choiceLabeling;
                
                // A vector that stores which states are markovian.
                boost::optional<storm::storage::BitVector> markovianStates;
            };
            
            struct Options {
                /*!
                 * Creates an object representing the default building options.
                 */
                Options();
                
                // The order in which to explore the model.
                ExplorationOrder explorationOrder;
                
                // A flag that indicates whether or not to store the state information after successfully building the
                // model. If it is to be preserved, it can be retrieved via the appropriate methods after a successful
                // call to <code>translateProgram</code>.
                bool buildStateValuations;
            };
            
            /*!
             * Creates an explicit model builder that uses the provided generator.
             *
             * @param generator The generator to use.
             */
            ExplicitModelBuilder(std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> const& generator, Options const& options = Options());

            /*!
             * Creates an explicit model builder for the given PRISM program.
             *
             * @param program The program for which to build the model.
             */
            ExplicitModelBuilder(storm::prism::Program const& program, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions(), Options const& builderOptions = Options());

            /*!
             * Creates an explicit model builder for the given JANI model.
             *
             * @param model The JANI model for which to build the model.
             */
            ExplicitModelBuilder(storm::jani::Model const& model, storm::generator::NextStateGeneratorOptions const& generatorOptions = storm::generator::NextStateGeneratorOptions(), Options const& builderOptions = Options());
            
            /*!
             * Convert the program given at construction time to an abstract model. The type of the model is the one
             * specified in the program. The given reward model name selects the rewards that the model will contain.
             *
             * @param program The program to translate.
             * @param constantDefinitionString A string that contains a comma-separated definition of all undefined
             * constants in the model.
             * @param rewardModel The reward model that is to be built.
             * @return The explicit model that was given by the probabilistic program.
             */
            std::shared_ptr<storm::models::sparse::Model<ValueType, RewardModelType>> build();
            
            /*!
             * If requested in the options, information about the variable valuations in the reachable states can be
             * retrieved via this function.
             *
             * @return A structure that stores information about all reachable states.
             */
            storm::storage::sparse::StateValuations const& getStateValuations() const;
            
        private:
            /*!
             * Retrieves the state id of the given state. If the state has not been encountered yet, it will be added to
             * the lists of all states with a new id. If the state was already known, the object that is pointed to by
             * the given state pointer is deleted and the old state id is returned. Note that the pointer should not be
             * used after invoking this method.
             *
             * @param state A pointer to a state for which to retrieve the index. This must not be used after the call.
             * @return A pair indicating whether the state was already discovered before and the state id of the state.
             */
            StateType getOrAddStateIndex(CompressedState const& state);
    
            /*!
             * Builds the transition matrix and the transition reward matrix based for the given program.
             *
             * @param transitionMatrixBuilder The builder of the transition matrix.
             * @param rewardModelBuilders The builders for the selected reward models.
             * @param choiceLabels is set to a vector containing the labels associated with each choice (is only set if choice labels are requested).
             * @param markovianChoices is set to a bit vector storing whether a choice is markovian (is only set if the model type requires this information).
             */
            void buildMatrices(storm::storage::SparseMatrixBuilder<ValueType>& transitionMatrixBuilder, std::vector<RewardModelBuilder<typename RewardModelType::ValueType>>& rewardModelBuilders, boost::optional<std::vector<boost::container::flat_set<uint_fast64_t>>>& choiceLabels , boost::optional<storm::storage::BitVector>& markovianChoices);
            
            /*!
             * Explores the state space of the given program and returns the components of the model as a result.
             *
             * @return A structure containing the components of the resulting model.
             */
            ModelComponents buildModelComponents();
            
            /*!
             * Set the markovian states of the given modelComponents,
             * makes sure that each state has at most one markovian choice,
             * and makes this choice the first one of the corresponding state
             *
             * @param modelComponents The components of the model build so far
             * @markovianChoices bit vector storing whether a choice is markovian
             */
            void buildMarkovianStates(ModelComponents& modelComponents, storm::storage::BitVector const& markovianChoices) const;
            
            /*!
             * Builds the state labeling for the given program.
             *
             * @return The state labeling of the given program.
             */
            storm::models::sparse::StateLabeling buildStateLabeling();
            
            /// The generator to use for the building process.
            std::shared_ptr<storm::generator::NextStateGenerator<ValueType, StateType>> generator;
            
            /// The options to be used for the building process.
            Options options;

            /// Internal information about the states that were explored.
            storm::storage::sparse::StateStorage<StateType> stateStorage;
            
            /// This member holds information about reachable states that can be retrieved from the outside after a
            /// successful build.
            boost::optional<storm::storage::sparse::StateValuations> stateValuations;
            
            /// A set of states that still need to be explored.
            std::deque<CompressedState> statesToExplore;
            
            /// An optional mapping from state indices to the row groups in which they actually reside. This needs to be
            /// built in case the exploration order is not BFS.
            boost::optional<std::vector<uint_fast64_t>> stateRemapping;

        };
        
    } // namespace adapters
} // namespace storm

#endif	/* STORM_BUILDER_EXPLICITMODELBUILDER_H */
