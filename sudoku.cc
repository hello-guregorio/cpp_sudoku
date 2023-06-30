#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;
// 输出路径
const char* output_path = "sudoku.txt";    // 存解
const char* endgame_path = "endgame.txt";  // 存终盘
const char* game_path = "game.txt";        // 存游戏
// some useful
std::unordered_set<char> opts;
int over_cnt;            // 终局数量
std::string input_path;  // 读取路径
int game_cnt;            // 需要游戏数量
int game_level = 1;      // 游戏等级
int hole_range_low = 20;
int hole_range_high = 25;
bool one_solution = false;  // 解唯一
random_device rd;
mt19937 rng(rd());
// 临时存放终盘
int EndGame[9][9];

// 解析命令行参数
int parse_opt(int argc, char** argv);
// 验证命令行参数合法性
int validate_opt();
void create_endgame();
void solve_sudoku(vector<vector<vector<int>>>& grids);
void save_endgame(int n);
bool find_item(char ch) { return opts.find(ch) != opts.end(); }
void gen_sudoku();
void write_game(ofstream& outfile, int grid[9][9]);
void solve_shudo(const char* filename);
vector<int> for_shuffle(81);
void init() {
  for (int i = 0; i < 81; ++i) {
    for_shuffle.push_back(i);
  }
}

int main(int argc, char** argv) {
  init();
  if (parse_opt(argc, argv) == -1) {
    return 0;
  }
  if (validate_opt() == -1) {
    return 0;
  }
  if (find_item('c')) {
    save_endgame(over_cnt);
  }
  if (find_item('s')) {
    solve_shudo(input_path.c_str());
  }
  if (find_item('n')) {
    gen_sudoku();
  }
}

/*----------------------------------------------------------------------------*/

bool validate_unique(int grid[9][9]) { return true; }

void copy_grid(int grid[9][9]) {
  for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 9; ++j) {
      grid[i][j] = EndGame[i][j];
    }
  }
}

void dig_hole(int grid[9][9], int hole_num) {
  shuffle(for_shuffle.begin(), for_shuffle.end(), rng);
  for (int i = 0; i < hole_num; ++i) {
    int row = for_shuffle[i] / 9;
    int col = for_shuffle[i] % 9;
    grid[row][col] = 0;
  }
}

void gen_sudoku() {
  create_endgame();
  ofstream outfile(game_path);
  for (int i = 0; i < game_cnt; ++i) {
    int hole_num = rand() % (hole_range_high + 1) + hole_range_low;
    int tmp_grid[9][9];
    copy_grid(tmp_grid);
    dig_hole(tmp_grid, hole_num);
    if (!one_solution) {
      write_game(outfile, tmp_grid);
    } else {
      int cnt = 0;
      while (!validate_unique(tmp_grid) && cnt++ < 10) {
        copy_grid(tmp_grid);
        dig_hole(tmp_grid, hole_num);
      }
      write_game(outfile, tmp_grid);
    }
  }
  outfile.close();
}

void write_game(ofstream& outfile, int grid[9][9]) {
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      outfile << grid[i][j] << " ";
    }
    outfile << endl;
  }
  outfile << endl;
}

void solve_sudoku(vector<vector<vector<int>>>& grids) { return; }

int validate_opt() {
  if (!find_item('n') && (find_item('m') || find_item('r') || find_item('u'))) {
    printf("lack opt n\n");
    return -1;
  }
  return 0;
}

int parse_opt(int argc, char** argv) {
  char opt_ch;
  const char* opt_string = "c:s:n:m:r:u";
  while ((opt_ch = getopt(argc, argv, opt_string)) != -1) {
    switch (opt_ch) {
      case 'c': {
        over_cnt = atoi(optarg);
        if (over_cnt < 1 || over_cnt > 1000000) {
          printf("-c out of range 1-1000000\n");
          return -1;
        }
        opts.insert('c');
        break;
      }

      case 's': {
        input_path = std::string(optarg);
        opts.insert('s');
        break;
      }
      case 'n': {
        opts.insert('n');
        game_cnt = atoi(optarg);
        if (game_cnt < 1 || game_cnt > 10000) {
          printf("-c out of range 1-1000000\n");
          return -1;
        }
        break;
      }

      case 'm': {
        opts.insert('m');
        game_level = atoi(optarg);
        if (game_level < 1 || game_level > 3) {
          printf("-m out of range 1-3\n");
          return -1;
        }
        break;
      }
      case 'r': {
        opts.insert('r');
        int idx = 0;
        int low = -1;
        int high = -1;
        while (optarg[idx] != '\0') {
          if (optarg[idx] == '~') {
            optarg[idx] = '\0';
            low = atoi(optarg);
            high = atoi(optarg + idx + 1);
            break;
          }
          idx++;
        }
        if (low == -1) {
          printf("invalid r arg\n");
          return -1;
        }
        if (low >= 20 && low <= 55 && high >= 20 && high <= 55 && low <= high) {
          hole_range_low = low;
          hole_range_high = high;
        } else {
          printf("-r out of range\n");
          return -1;
        }
        break;
      }

      case 'u': {
        opts.insert('u');
        one_solution = true;
        break;
      }
      default:
        printf("invalid opts\n");
        return -1;
    }
  }
  return 0;
}

void create_endgame() {
  bool is_use[10];  // 这一数组用于判断生成的随机数是否有重复的
  int num[9] = {0};  // 这一数组用于储存生成的随机数序列
  int sudoku[81];    // 这一数组用于储存最后生成的数独终盘
  memset(
      is_use, false,
      sizeof(
          is_use));  // 这一函数是用于将已经申请了空间的数组is_use中的值全部赋为false
  for (int i = 0; i < 9; i++) {
    srand(time(NULL));
    while (is_use[num[i]] ||
           num[i] == 0) {  // 这一语句用于判断生成的随机数是否是正确的，没有重复
      num[i] = rand() % 9 + 1;  // 这一语句用于生成一个1-9的随机数
    }
    is_use[num[i]] = true;
  }

  int tem[3][3],
      com[9][9] = {0},
      a = 0;  // tem数组用于储存生成的小九宫格，com数组用于储存大的九宫格
  for (int i = 0; i < 3;
       i++) {  // 这一嵌套for循环是用于将随机数列填到一个小九宫格中。
    for (int h = 0; h < 3; h++) {
      tem[i][h] = num[a];
      a++;
    }
  }

  for (int i = 0; i < 3;
       i++) {  // 这一嵌套for循环是用于将填好的小九宫格填到大的九宫格当中。
    for (int p = 0; p < 3; p++) {
      for (int h = 0; h < 3; h++) {
        com[3 * i + p][h] = tem[p][h];
        com[3 * i + p][h + 3] = tem[p][h];
        com[3 * i + p][h + 6] = tem[p][h];
      }
    }
  }

  for (int i = 0; i < 3;
       i++) {  // 这一嵌套for循环是用于截取需要用于生成数独终盘的数。
    for (int p = 0; p < 3; p++) {
      for (int k = 0; k < 3; k++) {
        for (int l = 0; l < 3; l++) {
          sudoku[27 * i + 9 * k + 3 * p + l] = com[k + p][l + i];
        }
      }
    }
  }

  int pp = 0;
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      EndGame[i][j] = sudoku[pp];
      pp++;
    }
  }
}
void save_endgame(int n) {
  ofstream outfile;
  outfile.open(endgame_path);
  for (int k = 0; k < n; k++) {
    bool is_use[10] = {0};  // 这一数组用于判断生成的随机数是否有重复的
    int num[9] = {0};      // 这一数组用于储存生成的随机数序列
    int sudoku[81] = {0};  // 这一数组用于储存最后生成的数独终盘
    memset(
        is_use, false,
        sizeof(
            is_use));  // 这一函数是用于将已经申请了空间的数组is_use中的值全部赋为false
    for (int i = 0; i < 9; i++) {
      srand(time(NULL));
      while (is_use[num[i]] ||
             num[i] ==
                 0) {  // 这一语句用于判断生成的随机数是否是正确的，没有重复
        num[i] = (rand() + k * 77) % 9 + 1;  // 这一语句用于生成一个1-9的随机数
      }
      is_use[num[i]] = true;
    }

    int tem[3][3],
        com[9][9] = {0},
        a = 0;  // tem数组用于储存生成的小九宫格，com数组用于储存大的九宫格
    for (int i = 0; i < 3;
         i++) {  // 这一嵌套for循环是用于将随机数列填到一个小九宫格中。
      for (int h = 0; h < 3; h++) {
        tem[i][h] = num[a];
        a++;
      }
    }

    for (int i = 0; i < 3;
         i++) {  // 这一嵌套for循环是用于将填好的小九宫格填到大的九宫格当中。
      for (int p = 0; p < 3; p++) {
        for (int h = 0; h < 3; h++) {
          com[3 * i + p][h] = tem[p][h];
          com[3 * i + p][h + 3] = tem[p][h];
          com[3 * i + p][h + 6] = tem[p][h];
        }
      }
    }

    for (int i = 0; i < 3;
         i++) {  // 这一套for循环是用于截取需要用于生成数独终盘的数。
      for (int p = 0; p < 3; p++) {
        for (int k = 0; k < 3; k++) {
          for (int l = 0; l < 3; l++) {
            sudoku[27 * i + 9 * k + 3 * p + l] = com[k + p][l + i];
          }
        }
      }
    }

    int pp = 0;
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        outfile << sudoku[pp];
        pp++;
      }
      outfile << endl;
    }
    outfile << endl;
  }
  outfile.close();
}

bool isValid(int row, int col, int val, vector<vector<int>>& board) {
  for (int i = 0; i < 9; i++) {  // 判断行里是否重复
    if (board[row][i] == val) {
      return false;
    }
  }
  for (int j = 0; j < 9; j++) {  // 判断列里是否重复
    if (board[j][col] == val) {
      return false;
    }
  }
  int startRow = (row / 3) * 3;
  int startCol = (col / 3) * 3;
  for (int i = startRow; i < startRow + 3; i++) {  // 判断9方格里是否重复
    for (int j = startCol; j < startCol + 3; j++) {
      if (board[i][j] == val) {
        return false;
      }
    }
  }
  return true;
}

bool backtracking(vector<vector<int>>& board) {
  for (int i = 0; i < board.size(); i++) {       // 遍历行
    for (int j = 0; j < board[0].size(); j++) {  // 遍历列
      if (board[i][j] == 0) {
        for (int k = 1; k <= 9; k++) {  // (i, j) 这个位置放k是否合适
          if (isValid(i, j, k, board)) {
            board[i][j] = k;                       // 放置k
            if (backtracking(board)) return true;  // 如果找到合适一组立刻返回
            board[i][j] = 0;                       // 回溯，撤销k
          }
        }
        return false;  // 9个数都试完了，都不行，那么就返回false
      }
    }
  }
  return true;  // 遍历完没有返回false，说明找到了合适棋盘位置了
}

void solve_shudo(const char* filename) {
  ifstream infile;
  infile.open(filename);
  if (!infile.is_open()) {
    return;
  }
  int read;
  vector<vector<vector<int>>> shudo;
  int shudon = 0;  // 数独数量
  int i = 0, j = 0;

  vector<vector<int>> t;
  for (int i = 0; i < 9; ++i) {
    vector<int> tmp(9, 0);
    t.push_back(tmp);
  }

  while (!infile.eof()) {
    infile >> read;
    if (read >= 0 && read <= 9) {
      t[i][j] = read;
      if (j + 1 == 9) {
        i++;
      }
      j = (j + 1) % 9;
      if (i == 9) {
        shudon++;
        i = j = 0;
        shudo.push_back(t);
        vector<vector<int>> t;
        for (int i = 0; i < 9; ++i) {
          vector<int> tmp(9, 0);
          t.push_back(tmp);
        }
      }
    }
  }

  infile.close();

  for (int k = 0; k < shudon; k++) {
    backtracking(shudo[k]);
  }

  ofstream outfile;
  outfile.open(output_path);

  for (int k = 0; k < shudon; k++) {
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        outfile << shudo[k][i][j] << " ";
      }
      outfile << endl;
    }
    outfile << endl;
  }
  outfile.close();
}