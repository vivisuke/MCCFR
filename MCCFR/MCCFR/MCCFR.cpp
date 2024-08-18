#include <iostream>
#include <random>
#include <numeric>
using namespace std;

enum {
    G = 0, C, P,
    N_GCP,
};
const char *hstr[] = {"Rock", "Scissors", "Paper"};
char gain_tbl[3][3] = {
    {0, 1, -1},
    {-1, 0, 1},
    {1, -1, 0},
};
const int MAX_REGRET_VAL = 10000;
int regrets_1[] = {0, 0, 0};		//	G, C, P 後悔値 for Player-1
int regrets_2[] = {0, 0, 0};		//	G, C, P 後悔値 for Player-2

std::random_device rnd;     		// 非決定的な乱数生成器
//std::mt19937 mt(rnd());				// メルセンヌ・ツイスタの32ビット版、引数は初期シード
std::mt19937 mt(0);
//std::uniform_int_distribution<> rand3(0, 2);        // [0, 99] 範囲の一様乱数

int next_hand(int *regrets) {		//	ジャンケンの次の手を決める
	int t[N_GCP];
	for(int i = 0; i != N_GCP; ++i) {
		t[i] = max(0, regrets[i]);
	}
	if( t[G] == 0 && t[C] == 0 && t[P] == 0 ) {
		return mt() % 3;
	}
	int sum = accumulate(t, t+N_GCP, 0);
	int r = mt() % sum;
	for(int i = 0; i != N_GCP; ++i) {
		if( r < t[i] ) return i;
		r -= t[i];
	}
	return -1;
}
void print() {
	cout << endl << "Player-1:" << endl;
	int sum = accumulate(&regrets_1[0], &regrets_1[N_GCP], 0);
	for(int i = 0; i != N_GCP; ++i) {
		//cout << "Regret Value = " << (double)regrets_1[i]/sum << " for " << hstr[i] << endl;
		cout << "probability = " << (double)regrets_1[i]/sum << " for " << hstr[i] << endl;
	}
	cout << endl << "Player-2:" << endl;
	sum = accumulate(&regrets_2[0], &regrets_2[N_GCP], 0);
	for (int i = 0; i != N_GCP; ++i) {
		//cout << "Regret Value = " << (double)regrets_2[i]/MAX_REGRET_VAL << " for " << hstr[i] << endl;
		cout << "probability = " << (double)regrets_2[i]/sum << " for " << hstr[i] << endl;
	}
	cout << endl;
}
int main()
{
	for (int i = 0; i != 1000000; ++i) {
		int h1 = next_hand(regrets_1);
		int h2 = next_hand(regrets_2);
		auto g1 = gain_tbl[h1][h2];
		if( false ) {
			cout << "h1 = " << hstr[h1] << endl;
			cout << "h2 = " << hstr[h2] << endl;
			if (g1 == 0) cout << "draw." << endl;
			else if (g1 > 0) cout << "p1 won." << endl;
			else if (g1 < 0) cout << "p2 won." << endl;
		}
		//	後悔値更新
		if (g1 < 0) {
			int mx = 0;
			for (int h = 0; h != N_GCP; ++h) {
				if (h == h1) continue;
				auto cr_g1 = gain_tbl[h][h2];
				mx = max(mx, (regrets_1[h] += cr_g1 - g1));
			}
			if( mx >= MAX_REGRET_VAL ) {
				for (int h = 0; h != N_GCP; ++h)
					regrets_1[h] /= 10;
			}
		}
		if (g1 > 0) {
			int mx = 0;
			for (int h = 0; h != N_GCP; ++h) {
				if (h == h2) continue;
				auto cr_g2 = gain_tbl[h][h1];
				mx = max(mx, (regrets_2[h] += cr_g2 + g1));
			}
			if( mx >= MAX_REGRET_VAL ) {
				for (int h = 0; h != N_GCP; ++h)
					regrets_2[h] /= 10;
			}
		}
		//if (i >= 100) {
		//	print();
		//}
	}
	print();

    std::cout << "\nOK." << endl;
}
