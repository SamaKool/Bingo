// Classic 5x5 Bingo (1-25) vs Computer
// Compile: g++ -std=c++11 -O2 -o bingo bingo.cpp
// Run:     ./bingo
//
// Rules:
//  * Each player has a 5x5 grid containing the numbers 1-25 exactly once.
//  * Players take turns calling a number that hasn't been called yet.
//    Every called number is struck out on BOTH grids.
//  * A completed row, column or diagonal is one line. The first player to
//    complete 5 lines (spelling B-I-N-G-O) wins.
//  * If both players reach 5 lines on the same call, it's a tie.
//
// Fairness: the computer's grid is random and stays hidden until the game
// ends. The computer chooses its numbers using ONLY its own grid and the list
// of numbers already called. It never looks at your grid.

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int N = 5;
const int TARGET = 5;

struct Grid {
    array<array<int, N>, N> num{};
    array<array<bool, N>, N> marked{};

    void randomize(mt19937 &rng) {
        vector<int> v(N * N);
        for (int i = 0; i < N * N; i++) v[i] = i + 1;
        shuffle(v.begin(), v.end(), rng);
        for (int i = 0; i < N * N; i++) num[i / N][i % N] = v[i];
        clearMarks();
    }

    void clearMarks() {
        for (auto &row : marked) row.fill(false);
    }

    void mark(int n) {
        for (int r = 0; r < N; r++)
            for (int c = 0; c < N; c++)
                if (num[r][c] == n) marked[r][c] = true;
    }

    // Each of the 12 lines as a list of 5 cells.
    static vector<array<pair<int, int>, N>> lines() {
        vector<array<pair<int, int>, N>> L;
        for (int i = 0; i < N; i++) {
            array<pair<int, int>, N> row, col;
            for (int j = 0; j < N; j++) {
                row[j] = {i, j};
                col[j] = {j, i};
            }
            L.push_back(row);
            L.push_back(col);
        }
        array<pair<int, int>, N> d1, d2;
        for (int i = 0; i < N; i++) {
            d1[i] = {i, i};
            d2[i] = {i, N - 1 - i};
        }
        L.push_back(d1);
        L.push_back(d2);
        return L;
    }

    int countMarkedIn(const array<pair<int, int>, N> &line) const {
        int k = 0;
        for (int i = 0; i < N; i++) k += marked[line[i].first][line[i].second];
        return k;
    }

    int completedLines() const {
        int done = 0;
        for (auto &line : lines())
            if (countMarkedIn(line) == N) done++;
        return done;
    }

    void print(const string &title) const {
        cout << "\n  " << title << "\n  +" << string(N * 5, '-') << "+\n";
        for (int r = 0; r < N; r++) {
            cout << "  |";
            for (int c = 0; c < N; c++) {
                if (marked[r][c])
                    cout << " [" << setw(2) << num[r][c] << "]";
                else
                    cout << "  " << setw(2) << num[r][c] << " ";
            }
            cout << "|\n";
        }
        cout << "  +" << string(N * 5, '-') << "+\n";
    }
};

static string progress(int lines) {
    string s;
    const string word = "BINGO";
    for (int i = 0; i < TARGET; i++) s += (i < lines ? word[i] : '_');
    return s;
}

// Read a full line from stdin; returns false on EOF.
static bool readLine(string &line) { return static_cast<bool>(getline(cin, line)); }

// Let the user type their own grid. Returns false on EOF.
static bool readCustomGrid(Grid &g) {
    cout << "\nEnter your 5x5 grid, one row per line, 5 numbers separated by spaces.\n"
         << "Use each number from 1 to 25 exactly once.\n";
    vector<int> vals;
    for (int r = 0; r < N; r++) {
        string line;
        while (true) {
            cout << "  Row " << (r + 1) << ": ";
            if (!readLine(line)) return false;
            istringstream ss(line);
            vector<int> row;
            int x;
            while (ss >> x) row.push_back(x);
            if (!ss.eof() || static_cast<int>(row.size()) != N) {
                cout << "  Please enter exactly 5 numbers.\n";
                continue;
            }
            bool ok = true;
            for (int v : row) {
                if (v < 1 || v > 25) {
                    cout << "  " << v << " is out of range (1-25).\n";
                    ok = false;
                    break;
                }
                if (find(vals.begin(), vals.end(), v) != vals.end() ||
                    count(row.begin(), row.end(), v) > 1) {
                    cout << "  " << v << " is a duplicate.\n";
                    ok = false;
                    break;
                }
            }
            if (!ok) continue;
            vals.insert(vals.end(), row.begin(), row.end());
            break;
        }
    }
    for (int i = 0; i < N * N; i++) g.num[i / N][i % N] = vals[i];
    g.clearMarks();
    return true;
}

// Computer's choice, based only on its own grid and which numbers are called.
static int computerPick(const Grid &cg, const array<bool, 26> &called, mt19937 &rng) {
    auto L = Grid::lines();
    int bestScore = -1;
    vector<int> best;
    for (int n = 1; n <= 25; n++) {
        if (called[n]) continue;
        int score = 0;
        for (auto &line : L) {
            bool contains = false;
            for (int i = 0; i < N; i++)
                if (cg.num[line[i].first][line[i].second] == n) contains = true;
            if (!contains) continue;
            int k = cg.countMarkedIn(line);
            score += (k + 1) * (k + 1);   // favour lines that are nearly done
            if (k == N - 1) score += 50;  // completes a line right now
        }
        if (score > bestScore) {
            bestScore = score;
            best = {n};
        } else if (score == bestScore) {
            best.push_back(n);
        }
    }
    return best[uniform_int_distribution<int>(0, best.size() - 1)(rng)];
}

int main() {
    random_device rd;
    mt19937 rng(rd());

    Grid player, computer;
    computer.randomize(rng);

    cout << "===== BINGO (1-25) vs COMPUTER =====\n"
         << "Take turns calling numbers. First to complete 5 lines wins.\n"
         << "\nHow do you want your grid?\n  1) Random\n  2) I'll type my own\nChoice: ";
    string line;
    if (!readLine(line)) return 0;
    if (!line.empty() && line[0] == '2') {
        if (!readCustomGrid(player)) return 0;
    } else {
        player.randomize(rng);
    }

    array<bool, 26> called{};
    called.fill(false);
    int callCount = 0;
    bool playerTurn = true;
    string lastMsg;

    while (callCount < 25) {
        player.print("YOUR GRID");
        cout << "\n  You: " << progress(player.completedLines()) << " (" << player.completedLines()
             << " lines)   Computer: " << progress(computer.completedLines()) << " ("
             << computer.completedLines() << " lines)\n";
        if (!lastMsg.empty()) cout << "  " << lastMsg << "\n";

        int n = 0;
        if (playerTurn) {
            while (true) {
                cout << "\n  Your turn. Call a number (1-25), or q to quit: ";
                if (!readLine(line)) return 0;
                if (!line.empty() && (line[0] == 'q' || line[0] == 'Q')) {
                    cout << "\nYou quit. The computer's grid was:\n";
                    computer.print("COMPUTER'S GRID");
                    return 0;
                }
                istringstream ss(line);
                int x;
                if (!(ss >> x) || x < 1 || x > 25) {
                    cout << "  Enter a number from 1 to 25.\n";
                    continue;
                }
                if (called[x]) {
                    cout << "  " << x << " was already called.\n";
                    continue;
                }
                n = x;
                break;
            }
            lastMsg = "You called " + to_string(n) + ".";
        } else {
            n = computerPick(computer, called, rng);
            lastMsg = "Computer called " + to_string(n) + ".";
        }

        called[n] = true;
        callCount++;
        player.mark(n);
        computer.mark(n);

        bool pw = player.completedLines() >= TARGET;
        bool cw = computer.completedLines() >= TARGET;
        if (pw || cw) {
            player.print("YOUR GRID");
            cout << "\n  " << lastMsg << "\n";
            cout << "\n=========================================\n";
            if (pw && cw) cout << "  BINGO for both at the same time - it's a tie!\n";
            else if (pw) cout << "  BINGO! You win!\n";
            else cout << "  The computer got BINGO first. You lose!\n";
            cout << "=========================================\n";
            computer.print("COMPUTER'S GRID (revealed)");
	    cin.get();
        }
        playerTurn = !playerTurn;
    }
    return 0;
}
