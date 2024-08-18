#include <iostream>
#include <random>
#include <vector>
#include <cmath>

using namespace std;

// ジャンケンの結果を表す列挙型
enum class JankenResult { WIN, LOSE, DRAW };

// ジャンケンの手を表す列挙型
enum class JankenHand { ROCK, PAPER, SCISSORS };

// プレイヤーの行動を表す構造体
struct PlayerAction {
    JankenHand hand;
    double regret;
    double strategy;
};

// モンテカルロCFRアルゴリズムを実装する関数
void monteCarloRegretMatching(vector<PlayerAction>& player1, vector<PlayerAction>& player2, int iterations) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < iterations; i++) {
        // 各プレイヤーの行動を決定
        JankenHand player1Hand = static_cast<JankenHand>(static_cast<int>(dis(gen) * 3));
        JankenHand player2Hand = static_cast<JankenHand>(static_cast<int>(dis(gen) * 3));

        // ジャンケンの結果を判定
        JankenResult result;
        if (player1Hand == player2Hand) {
            result = JankenResult::DRAW;
        }
        else if ((player1Hand == JankenHand::ROCK && player2Hand == JankenHand::SCISSORS) ||
            (player1Hand == JankenHand::PAPER && player2Hand == JankenHand::ROCK) ||
            (player1Hand == JankenHand::SCISSORS && player2Hand == JankenHand::PAPER)) {
            result = JankenResult::WIN;
        }
        else {
            result = JankenResult::LOSE;
        }

        // 各プレイヤーの後悔値を更新
        double util = (result == JankenResult::WIN) ? 1.0 : (result == JankenResult::LOSE) ? -1.0 : 0.0;
        player1[static_cast<int>(player1Hand)].regret += util - player1[static_cast<int>(player1Hand)].strategy;
        player2[static_cast<int>(player2Hand)].regret += -util - player2[static_cast<int>(player2Hand)].strategy;
    }

    // 各プレイヤーの最適戦略を計算
    double player1Sum = 0.0, player2Sum = 0.0;
    for (auto& action : player1) {
        action.strategy = max(action.regret, 0.0);
        player1Sum += action.strategy;
    }
    for (auto& action : player2) {
        action.strategy = max(action.regret, 0.0);
        player2Sum += action.strategy;
    }
    for (auto& action : player1) {
        action.strategy /= player1Sum;
    }
    for (auto& action : player2) {
        action.strategy /= player2Sum;
    }
}

int main() {
    // プレイヤー1の行動を表す配列
    vector<PlayerAction> player1 = {
        {JankenHand::ROCK, 0.0, 1.0 / 3.0},
        {JankenHand::PAPER, 0.0, 1.0 / 3.0},
        {JankenHand::SCISSORS, 0.0, 1.0 / 3.0}
    };

    // プレイヤー2の行動を表す配列
    vector<PlayerAction> player2 = {
        {JankenHand::ROCK, 0.0, 1.0 / 3.0},
        {JankenHand::PAPER, 0.0, 1.0 / 3.0},
        {JankenHand::SCISSORS, 0.0, 1.0 / 3.0}
    };

    // モンテカルロCFRアルゴリズムを実行
    //monteCarloRegretMatching(player1, player2, 10);
    monteCarloRegretMatching(player1, player2, 1000000);

    // 最適戦略を出力
    cout << "プレイヤー1の最適戦略:" << endl;
    for (const auto& action : player1) {
        cout << "手: " << static_cast<int>(action.hand) << ", 確率: " << action.strategy << endl;
    }
    cout << "プレイヤー2の最適戦略:" << endl;
    for (const auto& action : player2) {
        cout << "手: " << static_cast<int>(action.hand) << ", 確率: " << action.strategy << endl;
    }

    return 0;
}