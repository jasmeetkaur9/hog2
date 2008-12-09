#ifndef DSCOVER_H
#define DSCOVER_H

#include <vector>
//#include <utility>
#include <queue>
#include <ext/hash_set>
#include <ext/hash_map>
#include "MultiAgentEnvironment.h"
#include "GraphEnvironment.h"
#include "Map2DEnvironment.h"
#include "MyHash.h"
#include "DSCREnvironment.h"

/*
	Implementation of Cover Heuristic for different speed system

	note: this is only for one cop and one robber, the DS interface
		does support one cop only!
	note: TPDijkstra actually computes cover and a little bit more.
	note: in this implementation players are always allowed to pass their turn
*/
template<class state,class action>
class DSCover {

	public:

	typedef typename MultiAgentEnvironment<state,action>::MAState CRState;

	// constructor & destructor
	DSCover( SearchEnvironment<state,action> *env, unsigned int cop_speed = 1 );
	~DSCover();

	// who indicates who the cover was computed for
	// who == false => robber
	// who == true  => cop
	unsigned int cover( state pos_robber, state pos_cop, bool minFirst, bool &who );

	unsigned int nodesExpanded, nodesTouched;

	protected:

	DSCREnvironment<state,action> *dscrenv;

	void clear_cache();

	private:

	// queues and hash sets/maps
	class QueueEntry {
		public:
		QueueEntry() {};
		QueueEntry( state &_s, unsigned int _numTurns, state &_parent ):
			s(_s),numTurns(_numTurns),parent(_parent) {};
		state s;
		unsigned int numTurns;
		state parent;
	};

	struct QueueEntryCompare {
		bool operator() ( const QueueEntry &q1, const QueueEntry &q2 ) const {
			return( q1.numTurns > q2.numTurns );
		}
	};

	struct MyStateHash {
		size_t operator() ( const state s ) const {
			return StateHash<state>( s );
		}
	};

	typedef std::priority_queue<QueueEntry,std::vector<QueueEntry>,QueueEntryCompare> MyPriorityQueue;
	typedef __gnu_cxx::hash_set<state,MyStateHash> ClosedList;

	MyPriorityQueue robber_queue, cop_queue;
	ClosedList robber_closed, cop_closed;

};


/*------------------------------------------------------------------------------
| IMPLEMENTATION
------------------------------------------------------------------------------*/

template<class state,class action>
DSCover<state,action>::DSCover( SearchEnvironment<state,action> *env, unsigned int cop_speed ):
	dscrenv( new DSCREnvironment<state,action>( env, false, cop_speed ) )
{};

template<class state,class action>
DSCover<state,action>::~DSCover() {
	delete dscrenv;
};

template<class state,class action>
void DSCover<state,action>::clear_cache() {
	robber_queue = MyPriorityQueue();
	cop_queue    = MyPriorityQueue();
	robber_closed.clear();
	cop_closed.clear();
	return;
};

template<class state,class action>
unsigned int DSCover<state,action>::cover( state pos_robber, state pos_cop, bool minFirst, bool &who ) {

	if( pos_robber == pos_cop ) {
		who = false;
		return 0;
	}

	nodesExpanded = 0; nodesTouched = 0;

	// push the robber and the cop onto their open queues
	QueueEntry qcop( pos_cop, 0, pos_cop );
	cop_queue.push( qcop );
	QueueEntry qrobber( pos_robber, 0, pos_robber );
	robber_queue.push( qrobber );

	// create some variables that will be needed
	std::vector<state> neighbors;
	typename std::vector<state>::iterator it;
	QueueEntry qtemp;
	bool last_time_popped_from_robber_queue = true;

	// the counters for covered states
	unsigned int robber_cover = 0;
	unsigned int cop_cover    = 0;

	// as long as both players still have nodes to expand
	while( !robber_queue.empty() && !cop_queue.empty() ) {

		if( last_time_popped_from_robber_queue )
			qrobber = robber_queue.top();
		else
			qcop    = cop_queue.top();

		nodesTouched++;

		if( minFirst?(qrobber.numTurns<qcop.numTurns):(qrobber.numTurns<=qcop.numTurns) ) {
			// robber makes his move
			robber_queue.pop();
			last_time_popped_from_robber_queue = true;

			if( robber_closed.find( qrobber.s ) == robber_closed.end() &&
			    cop_closed.find( qrobber.parent ) == cop_closed.end() ) {
			    //cop_closed.find( qrobber.s ) == cop_closed.end() ) {

				// verbose
				//printf( "expanded node %lu for the robber\n", qrobber.s );

				nodesExpanded++;
				robber_cover++;
				robber_closed.insert( qrobber.s );

				dscrenv->GetRobberSuccessors( qrobber.s, neighbors );

				for( it = neighbors.begin(); it != neighbors.end(); it++ ) {
					nodesTouched++;
					qtemp.s        = *it;
					qtemp.numTurns = qrobber.numTurns + 1;
					qtemp.parent   = qrobber.s;
					robber_queue.push( qtemp );
					//printf( "pushing %lu onto the queue for the robber\n", qtemp.s );
				}
			}

		} else {
			// cop makes his move
			cop_queue.pop();
			last_time_popped_from_robber_queue = false;

			if( cop_closed.find( qcop.s ) == cop_closed.end() ) {

				// verbose
				//printf( "expanded node %lu for the cop\n", qcop.s );

				nodesExpanded++;
				cop_closed.insert( qcop.s );

				// whether or not this state should be counted towards the cop cover
				// this is the case when the cop is neither capturing the robber
				// (in this case the node is counted towards the robber)
				// and when the state is adjacent to a robbers position (then it
				// will be counted towards the robber cover in the next turn).
				bool count_this_node = true;
				if( robber_closed.find( qcop.s ) != robber_closed.end() )
					count_this_node = false;

				dscrenv->GetCopSuccessors( qcop.s, neighbors );

				for( it = neighbors.begin(); it != neighbors.end(); it++ ) {
					nodesTouched++;
					if( robber_closed.find( *it ) == robber_closed.end() ) {
						qtemp.s        = *it;
						qtemp.numTurns = qcop.numTurns + 1;
						//qtemp.parent   = qcop.s; // not needed
						cop_queue.push( qtemp );
						//printf( "pushing %lu onto the queue for the cop\n", qtemp.s );
					} else {
						count_this_node = false;
					}
				}

				if( count_this_node ) cop_cover++;
			}
		}
	}

	unsigned int result;
	// determine the return values
	if( robber_queue.empty() ) {
		who    = false;
		result = robber_cover;
	} else {
		who    = true;
		result = cop_cover;
	}

	// cleanup
	clear_cache();

	return result;
};

#endif