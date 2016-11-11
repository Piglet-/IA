// Game of Othello -- Example of main
// Universidad Simon Bolivar, 2012.
// Author: Blai Bonet
// Last Revision: 1/11/16
// Modified by: 

#include <iostream>
#include <limits>
#include <climits>
#include "othello_cut.h" // won't work correctly until .h is fixed!
#include "utils.h"
#include <list>
#include <unordered_map>

using namespace std;

unsigned expanded = 0;
unsigned generated = 0;
int tt_threshold = 32; // threshold to save entries in TT

// Transposition table
struct stored_info_t {
    int value_;
    int type_;
    enum { EXACT, LOWER, UPPER };
    stored_info_t(int value = -100, int type = LOWER) : value_(value), type_(type) { }
};

struct hash_function_t {
    size_t operator()(const state_t &state) const {
        return state.hash();
    }
};

class hash_table_t : public unordered_map<state_t, stored_info_t, hash_function_t> {
};

hash_table_t TTable[2];

//Funcion que retorna una lista de hijos del estado
list <state_t> get_children(state_t state,bool color_player){
	list <state_t> children;
	
	for( int pos = 0; pos < DIM; ++pos ) {
            if(state.outflank(color_player,pos)) {
                state_t new_state = state.move(color_player,pos);
				children.push_back(new_state);
            }
    }
	return children;	
}

//Funcion minimo
int min(int score, int maxmin_result){
	if (maxmin_result < score){
		return maxmin_result;
	}
	return score;
}

//Funcion maximo
int max(int score, int minmax_result){
	if (minmax_result > score){
		return minmax_result;
	}
	return score;
}

//Definicion abstracta de maxmin
int maxmin(state_t state, int depth, bool use_tt);

//Funcion minmax
int minmax(state_t state, int depth, bool use_tt){

	++expanded;	

	if (state.terminal())
		return state.value();

	int score = INT_MAX;	
	list <state_t> children = get_children(state,false);
	
	if (children.empty()) {
		score = min(score, maxmin(state, depth - 1, use_tt));
    }
	else {
	   list <state_t>::iterator child = children.begin();
	   while(child != children.end()){
		    score = min(score, maxmin(*child,depth - 1,use_tt));
		    ++child;
	    }
    }
	return score;		
}

//Funcion maxmin
int maxmin(state_t state, int depth, bool use_tt){

	++expanded;

	if (state.terminal())
		return state.value();

	int score = INT_MIN;	
	list <state_t> children = get_children(state,true);

    if (children.empty()) {
        ++generated;
        score = max(score, minmax(state, depth - 1, use_tt));
    }
    else {
	   list <state_t>::iterator child = children.begin();
	   while(child != children.end()){
            score = max(score, minmax(*child,depth - 1,use_tt));
		    ++child;
	    }
    }
	return score;		
}

int negamax(state_t state, int depth, int color, bool use_tt = false);
int negamax(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false);
int scout(state_t state, int depth, int color, bool use_tt = false);
int negascout(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false);

int main(int argc, const char **argv) {
    state_t pv[128];
    int npv = 0;
    for( int i = 0; PV[i] != -1; ++i ) ++npv;

    int algorithm = 0;
    if( argc > 1 ) algorithm = atoi(argv[1]);
    bool use_tt = argc > 2;

    // Extract principal variation of the game
    state_t state;
    cout << "Extracting principal variation (PV) with " << npv << " plays ... " << flush;
    for( int i = 0; PV[i] != -1; ++i ) {
        bool player = i % 2 == 0; // black moves first!
        int pos = PV[i];
        pv[npv - i] = state;
        state = state.move(player, pos);
    }
    pv[0] = state;
    cout << "done!" << endl;

	//if (0){
    	// print principal variation
    //	for( int i = 0; i <= npv; ++i )
    //    	cout << pv[npv - i];
	//}

    // Print name of algorithm
    cout << "Algorithm: ";
    if( algorithm == 0 ) {
        cout << "Minmax-Maxmin";
    } else if( algorithm == 1 ) {
        cout << "Negamax (minmax version)";
    } else if( algorithm == 2 ) {
        cout << "Negamax (alpha-beta version)";
    } else if( algorithm == 3 ) {
        cout << "Scout";
    } else if( algorithm == 4 ) {
        cout << "Negascout";
    }
    cout << (use_tt ? " w/ transposition table" : "") << endl;

    // Run algorithm along PV (bacwards)
    cout << "Moving along PV:" << endl;
    for( int i = 0; i <= npv; ++i ) {
        //cout << pv[i];
        int value = 0;
        TTable[0].clear();
        TTable[1].clear();
        float start_time = Utils::read_time_in_seconds();
        expanded = 0;
        generated = 0;
        int color = i % 2 == 1 ? 1 : -1;

        try {
            if( algorithm == 0 ) {
                value = color * (color == 1 ? maxmin(pv[i], 0, use_tt) : minmax(pv[i], 0, use_tt));
            } else if( algorithm == 1 ) {
                //value = negamax(pv[i], 0, color, use_tt);
            } else if( algorithm == 2 ) {
                //value = negamax(pv[i], 0, -200, 200, color, use_tt);
            } else if( algorithm == 3 ) {
                //value = scout(pv[i], 0, color, use_tt);
            } else if( algorithm == 4 ) {
                //value = negascout(pv[i], 0, -200, 200, color, use_tt);
            }
        } catch( const bad_alloc &e ) {
            cout << "size TT[0]: size=" << TTable[0].size() << ", #buckets=" << TTable[0].bucket_count() << endl;
            cout << "size TT[1]: size=" << TTable[1].size() << ", #buckets=" << TTable[1].bucket_count() << endl;
            use_tt = false;
        }

        float elapsed_time = Utils::read_time_in_seconds() - start_time;

        cout << npv + 1 - i << ". " << (color == 1 ? "Black" : "White") << " moves: "
             << "value=" << color * value
             << ", #expanded=" << expanded
             << ", #generated=" << generated
             << ", seconds=" << elapsed_time
             << ", #generated/second=" << generated/elapsed_time
             << endl;
    }

    return 0;
}
