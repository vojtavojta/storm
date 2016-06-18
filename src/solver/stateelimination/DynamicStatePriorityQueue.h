#pragma once

#include <functional>
#include <vector>
#include <set>
#include <unordered_map>

#include "src/solver/stateelimination/StatePriorityQueue.h"

namespace storm {
    namespace storage {
        template<typename ValueType>
        class FlexibleSparseMatrix;
    }
    
    namespace solver {
        namespace stateelimination {
                        
            struct PriorityComparator {
                bool operator()(std::pair<storm::storage::sparse::state_type, uint_fast64_t> const& first, std::pair<storm::storage::sparse::state_type, uint_fast64_t> const& second) {
                    return (first.second < second.second) || (first.second == second.second && first.first < second.first) ;
                }
            };
            
            template<typename ValueType>
            class DynamicStatePriorityQueue : public StatePriorityQueue {
            public:
                typedef std::function<uint_fast64_t (storm::storage::sparse::state_type const& state, storm::storage::FlexibleSparseMatrix<ValueType> const& transitionMatrix, storm::storage::FlexibleSparseMatrix<ValueType> const& backwardTransitions, std::vector<ValueType> const& oneStepProbabilities)> PenaltyFunctionType;
                
                DynamicStatePriorityQueue(std::vector<std::pair<storm::storage::sparse::state_type, uint_fast64_t>> const& sortedStatePenaltyPairs, storm::storage::FlexibleSparseMatrix<ValueType> const& transitionMatrix, storm::storage::FlexibleSparseMatrix<ValueType> const& backwardTransitions, std::vector<ValueType> const& oneStepProbabilities, PenaltyFunctionType const& penaltyFunction);
                
                virtual bool hasNext() const override;
                virtual storm::storage::sparse::state_type pop() override;
                virtual void update(storm::storage::sparse::state_type state) override;
                virtual std::size_t size() const override;
                
            private:
                storm::storage::FlexibleSparseMatrix<ValueType> const& transitionMatrix;
                storm::storage::FlexibleSparseMatrix<ValueType> const& backwardTransitions;
                std::vector<ValueType> const& oneStepProbabilities;
                std::set<std::pair<storm::storage::sparse::state_type, uint_fast64_t>, PriorityComparator> priorityQueue;
                std::unordered_map<storm::storage::sparse::state_type, uint_fast64_t> stateToPriorityMapping;
                PenaltyFunctionType penaltyFunction;
            };
            
        }
    }
}