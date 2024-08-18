#include <iostream>
#include <vector>
#include <random>
#include <numeric>
using namespace std;

enum {
	PASS = 0,		//	弱気：Check or Fold
	RAISE,			//	強気：Raise or Call
	N_ACTION,
	J = 0, Q, K, N_RANK,
	S_INIT = 0, S_CHECKED, S_RAISED_1, S_RAISED_2,
};
const char* str_rank[] = {"J", "Q", "K"};
const char* str_move[] = {"Check", "Raise", "Fold", "Call"};
const int MAX_REGRET_VAL = 10000;
const int N_STATE = 4;
/*
	  init  Check Raise Raise
	-+-----+-----+-----+-----+
	J| [0] | [3] | [6] | [9] |
	Q| [1] | [4] | [7] | [10]|
	K| [2] | [5] | [8] | [11]|

*/
//	各状態・手札・アクションの後悔値、[弱気、強気]
int regrets[][2] = {
	{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
	{1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
};
vector<int> deck = {J, Q, K};

std::random_device rnd;     		// 非決定的な乱数生成器
//std::mt19937 mt(rnd());				// メルセンヌ・ツイスタの32ビット版、引数は初期シード
std::mt19937 mt(0);
//std::uniform_int_distribution<> rand3(0, 2);        // [0, 99] 範囲の一様乱数
void print_regrets() {
	printf("\n");
	printf("    init  Chkd  Rsd-1 Rsd-2\n");
	printf("  -+-----+-----+-----+-----+\n");
	for(int r = J; r <= K; ++r) {
		printf("  %s", str_rank[r]);
		int ix = r;
		for(int s = 0; s < N_STATE; ++s, ix+=N_RANK) {
			const int* ptr = regrets[ix];
			printf("%5d ", ptr[0]);
		}
		printf("\n");
		printf("   ");
		ix = r;
		for(int s = 0; s < N_STATE; ++s, ix+=N_RANK) {
			const int* ptr = regrets[ix];
			printf("%5d ", ptr[1]);
		}
		printf("\n");
	}
	printf("\n");
}
void print_table() {
	printf("\n");
	printf("Raise or Call prob. table:\n");
	printf("    init  Chkd  Rsd-1 Rsd-2\n");
	printf("  -+-----+-----+-----+-----+\n");
	for(int r = J; r <= K; ++r) {
		printf("  %s", str_rank[r]);
		int ix = r;
		for(int s = 0; s < N_STATE; ++s, ix+=N_RANK) {
			const int* ptr = regrets[ix];
#if 1
			int v[2];
			v[0] = max(0, ptr[0]);
			v[1] = max(0, ptr[1]);
			if( v[0] == 0 && v[1] == 0 )
				printf("  50%% ");
			else
				printf("%4d%% ", v[1]*100/(v[0]+v[1]));
#else
			if( ptr[0] == 0 && ptr[1] == 0 )
				printf("  50%% ");
			else
				printf("%4d%% ", ptr[1]*100/(ptr[0]+ptr[1]));
#endif
		}
		printf("\n");
	}
	printf("\n");
}
int next_hand(int* regrets) {		//	次の手を決める
	int t[N_ACTION];
	for (int i = 0; i != N_ACTION; ++i) {
		t[i] = max(0, regrets[i]);
	}
	if (t[PASS] == 0 && t[RAISE] == 0 ) {
		return mt() % 2;
	}
	int sum = accumulate(t, t + N_ACTION, 0);
	int r = mt() % sum;
	for (int i = 0; i != N_ACTION; ++i) {
		if (r < t[i]) return i;
		r -= t[i];
	}
	return -1;
}
int showdown() {
	return deck[0] > deck[1] ? 1 : -1;
}
//	初手は depth: 0 でコールされる
int playout(int depth=0, bool raised=false, bool learn=true) {
	int gain = 0;		//	先手利得
	int cfg = 0;		//	反事実利得
	if( depth == 0 ) {
		int m1 = next_hand(regrets[S_INIT*N_RANK+deck[0]]);
		if( learn )
			cout << "move1 = " << str_move[m1] << endl;
		gain = playout(depth+1, m1==RAISE, learn);
		if( learn ) {
			int cfm = (PASS+RAISE) - m1;
			cfg = playout(depth+1, cfm==RAISE, false);
			regrets[S_INIT*N_RANK+deck[0]][cfm] += cfg - gain;
		}
	} else if( depth == 1 ) {
		if( !raised ) {	//	初手 Check の場合
			int m2 = next_hand(regrets[S_CHECKED*N_RANK+deck[1]]);
			if( learn )
				cout << "move2 = " << str_move[m2] << endl;
			if( m2 == PASS ) {		//	後手 Check -> ショーダウン
				gain = showdown();
			} else {	//	後手 Raise
				gain = playout(depth+1, true, learn);
			}
			if( learn ) {
				int cfm = (PASS+RAISE) - m2;
				//int cfg = playout(depth+1, cfm==RAISE, false);
				if( m2 == PASS ) {		//	後手 Check -> ショーダウン
					cfg = showdown();
				} else {	//	後手 Raise
					cfg = playout(depth+1, true, false);
				}
				regrets[S_CHECKED*N_RANK+deck[1]][cfm] += gain - cfg;
			}
		} else {		//	初手 Raise の場合
			int m2 = next_hand(regrets[S_RAISED_1*N_RANK+deck[1]]);
			if( learn )
				cout << "move2 = " << str_move[m2+N_ACTION] << endl;
			if( m2 == PASS ) {		//	後手 Fold
				gain = 1;
			} else {		//	後手 Call -> ショーダウン
				gain = showdown() * 2;
			}
			if( learn ) {
				int cfm = (PASS+RAISE) - m2;
				if( cfm == PASS ) {		//	後手 Fold
					cfg = 1;
				} else {
					cfg = showdown() * 2;
				}
				//if( gain > cfg )
					regrets[S_RAISED_1* N_RANK +deck[1]][cfm] += gain - cfg;
			}
		}
	} else if( depth == 2 ) {	//	先手：Chaeck -> 後手：Raise の場合
		int m3 = next_hand(regrets[S_RAISED_2*N_RANK+deck[0]]);
		if( learn )
			cout << "move3 = " << str_move[m3+N_ACTION] << endl;
		if( m3 == PASS ) {		//	先手 Fold
			gain = -1;
		} else {
			gain = showdown() * 2;
		}
		if( learn ) {
			int cfm = (PASS+RAISE) - m3;
			if( cfm == PASS ) {		//	先手 Fold
				cfg = -1;
			} else {
				cfg = showdown() * 2;
			}
			//if( gain < cfg )
				regrets[S_RAISED_2*N_RANK+deck[0]][cfm] += cfg - gain;
		}
	}
	return gain;
}
#if 0
int playout(int stat, bool learn=true) {
	int gain = 0;
	if( stat == S_INIT ) {
		int m1 = next_hand(regrets[stat*N_RANK+deck[0]]);
		cout << "move1 = " << str_move[m1] << endl;
		if( m1 == PASS )
			gain = playout(S_CHECKED);
		else
			gain = playout(S_RAISED_1);
	} else if( stat == S_CHECKED ) {	//	初手 Check の場合
		int m2 = next_hand(regrets[stat*N_RANK+deck[1]]);
		cout << "move2 = " << str_move[m2] << endl;
		if( m2 == PASS ) {		//	後手 Check -> ショーダウン
			gain = showdown();
		} else {	//	後手 Raise
			gain = playout(S_RAISED_2);
		}
	} else if( stat == S_RAISED_1 ) {	//	初手 Raise の場合
		int m2 = next_hand(regrets[stat*N_RANK+deck[1]]);
		cout << "move2 = " << str_move[m2+N_ACTION] << endl;
		if( m2 == PASS ) {		//	後手 Fold
			gain = 1;
		} else {		//	後手 Call -> ショーダウン
			gain = showdown() * 2;
		}
	} else if( stat == S_RAISED_2 ) {	//	Chaeck -> Raise の場合
		int ix = stat*N_RANK+deck[0];
		int m3 = next_hand(regrets[ix]);
		cout << "move3 = " << str_move[m3+N_ACTION] << endl;
		if( m3 == PASS ) {		//	先手 Fold
			gain = -1;
		} else {
			gain = showdown() * 2;
		}
		if( learn ) {
			int cfm3 = (PASS+RAISE) - m3;
			int cfg;
			if( cfm3 == PASS ) {		//	先手 Fold
				cfg = -1;
			} else {
				cfg = showdown() * 2;
			}
			if( gain < cfg )
				regrets[stat*N_RANK+deck[1]][cfm3] += cfg - gain;
		}
	}
	return gain;
}
#endif
int main()
{
	for(int i = 0; i != 100; ++i) {
		shuffle(deck.begin(), deck.end(), mt);
		cout << str_rank[deck[0]] << " " << str_rank[deck[1]] << endl;
		//int gain = playout(S_INIT);
		int gain = playout();
		cout << "gain = " << gain << endl;
	    print_regrets();
	    print_table();
	}
    //print_table();
#if 0
	int gain = 0;	//	先手利得
	int stat = S_INIT;
	int m1 = next_hand(regrets[stat*N_RANK+deck[0]]);
	cout << "move1 = " << str_move[m1] << endl;
	stat += m1+1;
	int m2 = next_hand(regrets[stat*N_RANK+deck[1]]);
	if( stat == S_CHECKED ) {	//	初手 Check の場合
		cout << "move2 = " << str_move[m2] << endl;
		if( m2 == PASS ) {		//	後手 Check -> ショーダウン
			if( deck[0] > deck[1] )
				gain = 1;
			else
				gain = -1;
		} else {	//	後手が Raise した場合
			stat = S_RAISED_2;
			int m3 = next_hand(regrets[stat*N_RANK+deck[0]]);
			cout << "move3 = " << str_move[m3+N_ACTION] << endl;
			if( m3 == PASS ) {		//	先手 Fold
				gain = -1;
			} else {
				if( deck[0] > deck[1] )
					gain = 2;
				else
					gain = -2;
			}
		}
	} else {	//	初手 Raise の場合
		cout << "move2 = " << str_move[m2+N_ACTION] << endl;
		if( m2 == PASS ) {		//	後手 Fold
			gain = 1;
		} else {
			if( deck[0] > deck[1] )
				gain = 2;
			else
				gain = -2;
		}
	}
	cout << "gain = " << gain << endl;
#endif

    std::cout << endl << "OK." << endl;
}
