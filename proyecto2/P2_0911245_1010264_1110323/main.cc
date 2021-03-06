// Game of Othello -- Example of main
// Universidad Simon Bolivar, 2012.
// Author: Blai Bonet
// Last Revision: 1/11/16
// Modified by: 
//        * Chiseng Ng Yu               09-11245
//        * María Lourdes Garcia Florez 10-10264
//        * Carlos Ferreira             11-10323

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

//Function mininum
int min(int score, int maxmin_result){
	if (maxmin_result < score){
		return maxmin_result;
	}
	return score;
}

//Function maximum
int max(int score, int minmax_result){
	if (minmax_result > score){
		return minmax_result;
	}
	return score;
}

//Forward declaretion of maxmin
int maxmin(state_t state, int depth, bool use_tt);

//Function minmax
int minmax(state_t state, int depth, bool use_tt){

	bool empty = true; // Check if some child was generated
    state_t child;     // State for create the new child

    if (state.terminal())
		return state.value();

    ++expanded; 

	int score = INT_MAX;
	
	for ( int pos = 0; pos < DIM; ++pos ){
        if (state.outflank(false,pos)){
            empty = false;
            ++generated;
            child = state.move(false,pos);
		    score = min(score, maxmin(child,depth - 1,use_tt));
	    }
    }

    if (empty){     // No child was generate pass turn
        ++generated;
        score = min(score, maxmin(state, depth - 1, use_tt));
    }

	return score;		
}

//Function maxmin
int maxmin(state_t state, int depth, bool use_tt){

	bool empty = true;    
    state_t child;         

	if (state.terminal())
		return state.value();

    ++expanded;

    int score = INT_MIN;	

    for ( int pos = 0; pos < DIM; ++pos ){
        if (state.outflank(true,pos)){
            empty = false;
            ++generated;
            child = state.move(true,pos);
            score = max(score, minmax(child,depth - 1,use_tt));
        }
    }

    if (empty){     // No child was generate pass turn
        ++generated;
        score = max(score, minmax(state, depth - 1, use_tt));
    }

	return score;		
}

int negamax(state_t state, int depth, int color, bool use_tt = false){
	
	bool empty = true;             
    bool is_color = 1 == color;	   // Variable use for the next move

	if (state.terminal())
		return color * state.value();
	
	++expanded;

	int alpha = INT_MIN;

	state_t child;                 

    for( int pos = 0; pos < DIM; ++pos ) {
        if(state.outflank(is_color ,pos)) {
			++generated;
            empty = false;
            child = state.move(is_color,pos);
            alpha = max(alpha, -negamax(child, depth + 1, -color, use_tt));
        }
    }

    if (empty){     // No child was generate pass turn
		++generated;
		alpha = max(alpha, -negamax(state, depth + 1, -color, use_tt));
	}

	return alpha;
	
}

int negamax(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false){

	bool empty = true;             
    bool is_color = 1 == color;	   

	if (state.terminal())
		return color * state.value();

	++expanded;

	int score = INT_MIN;

	int val = 0;
	
	state_t child;                 

    for( int pos = 0; pos < DIM; ++pos ) {
        if(state.outflank(is_color ,pos)) {
			++generated;
            empty = false;
            child = state.move(is_color,pos);
            val = -negamax(child, depth + 1, -beta, -alpha, -color, use_tt);
			score = max(score,val);
			alpha = max(alpha,val);
			if (alpha >= beta)
				break;
        }
    }

    if (empty){     // No child was generate pass turn
		++generated;
		val = -negamax(state, depth + 1, -beta, -alpha, -color, use_tt);
        score = max(score,val);
	}
	return score;	
}

bool test(state_t state, int score, int color, bool condition, bool use_tt){

    if (state.terminal() && condition)
        return state.value() > score ? true : false;
    else if (state.terminal() && !condition) 
        return state.value() >= score ? true : false;

    bool is_color = color == 1;     
    bool empty = true;             
    state_t child;                  
    ++expanded;

    for( int pos = 0; pos < DIM; ++pos ) {
        if(state.outflank(is_color ,pos)) {
            empty = false;
            ++generated;
            child = state.move(is_color,pos);
            if (is_color && test(child, score, -color, condition, use_tt))
                return true;
            if (!is_color && !test(child, score, -color, condition, use_tt))
                return false;
        }
    }

    if (empty) {    // No child was generate pass turn
        ++generated;
        if (is_color && test(state, score, -color, condition, use_tt))
            return true;
        if (!is_color && !test(state, score, -color, condition, use_tt))
            return false;
    }

    return color == 1 ? false : true;
}

int scout(state_t state, int depth, int color, bool use_tt = false){

    bool f_child = true;            // Check if the child is the first one
    bool empty = true; 
    bool is_color = 1 == color;
    state_t child;

    if (state.terminal())
        return state.value();

    ++expanded;

    int score = 0;

    for( int pos = 0; pos < DIM; ++pos ) {
        if(state.outflank(is_color ,pos)) {
            empty = false;
            ++generated;
            child = state.move(is_color,pos);
            if (f_child) {
                f_child = false;
                score = scout(child, depth - 1, -color, use_tt);
            }
            else {
                if (is_color && test(child, score, -color, true, use_tt))
                    score = scout(child,depth - 1, -color, use_tt);
                if (!is_color && !test(child, score, -color, false, use_tt))
                    score = scout(child,depth - 1, -color, use_tt);
            }
        }
    }

    if (empty){
        ++generated;
        score = scout(state, depth - 1, -color, use_tt);
    }

    return score;
}

int negascout(state_t state, int depth, int alpha, int beta, int color, bool use_tt = false){

    bool f_child = true;    
    bool empty = true;
    bool is_color = 1 == color;
    int score = 0;
    state_t child;

    if (state.terminal())
        return color * state.value();

    ++expanded;

 
    for( int pos = 0; pos < DIM; ++pos ) {
        if(state.outflank(is_color ,pos)) {
            empty = false;
            ++generated;
            child = state.move(is_color,pos);
            if (f_child) {
                f_child = false;
                score = -negascout(child, depth - 1, -beta, -alpha, -color, use_tt);
            }
            else {
                score = -negascout(child, depth - 1, -alpha -1, -alpha, -color, use_tt);

                if (alpha < score && score < beta)
                    score = -negascout(child, depth - 1, -beta, -score, -color, use_tt);
            }
            
            alpha = max(alpha, score);
            if (alpha >= beta)
                break;
        }        
    }
    
    if (empty){         // No child was generate pass turn
        ++generated;
        score = -negascout(state, depth - 1, -beta, -alpha, -color, use_tt);
        alpha = max(alpha, score);
    }
    return alpha;

}

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
                value = negamax(pv[i], 0, color, use_tt);
            } else if( algorithm == 2 ) {
                value = negamax(pv[i], 0, -200, 200, color, use_tt);
            } else if( algorithm == 3 ) {
                value = color * scout(pv[i], 0, color, use_tt);
            } else if( algorithm == 4 ) {
                value = negascout(pv[i], 0, -200, 200, color, use_tt);
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

