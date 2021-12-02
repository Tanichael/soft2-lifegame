#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // sleep()関数を使う
#include <string.h>

//------------------------------------------------------------
// lifファイルの数字は 列 行 の順番
//------------------------------------------------------------
void my_init_cells(const int height, const int width, int cell[height][width], FILE* fp) {
  if(fp == NULL) {
    //NULLの時の処理
    //default.lif
    //#Life 1.06
    // 30 20
    // 30 22
    // 31 22
    // 31 23
    // 32 20
    cell[20][30] = 1;
    cell[22][30] = 1;
    cell[22][31] = 1;
    cell[23][31] = 1;
    cell[20][32] = 1;
  } else {
    char c;
    //fpの読み込み、値の代入
    //一応数値以外の値が入っていた時に対応するべき？
    //めっちゃ回りくどいけど... 空白の扱いで悩んだ結果以下のような感じ
    int x;
    int y;
    int cnt = 0;
    char s[10];
    const size_t bufsize = 500;
    char buf[bufsize];

    fgets(buf, bufsize, fp); //最初の行を飛ばす

    while((c = fgetc(fp)) != EOF) {
      if(c == ' ') { //半角スペースの時
        x = atoi(s);
        memset(s, '\0', sizeof(s));
        cnt = 0;
      } else if(c == '\n') { //改行の時
        y = atoi(s);
        memset(s, '\0', sizeof(s));
        cnt = 0;
        cell[y][x] = 1;
      } else if(c >= '0' && c <= '9'){ //数値の時
        s[cnt] = c;
        cnt++;
      }
    }
  }
}

//暫定
void my_print_cells(FILE* fp, int gen, const int height, const int width, int cell[height][width]) {
  fprintf(fp, "generation = %d\n", gen);
  fprintf(fp, "+");
  for(int x = 0; x < width; x++) {
    fprintf(fp, "-");
  }
  fprintf(fp, "+\n");

  for(int y = 0; y < height; y++) {
    fprintf(fp, "|");
    for(int x = 0; x < width; x++) {
      fprintf(fp, "\x1b[31m"); 
      if(cell[y][x] == 1) {
        fprintf(fp, "#");
      } else {
        fprintf(fp, " ");
      }
    }
    fprintf(fp, "\x1b[37m");
    fprintf(fp, "|");
    fprintf(fp, "\n");
  }

  fprintf(fp, "+");
  for(int x = 0; x < width; x++) {
    fprintf(fp, "-");
  }
  fprintf(fp, "+\n");

  fprintf(fp, "\r");
}

int my_count_adjacent_cells(int h, int w, const int height, const int width, int cell[height][width]) {
  int cnt = 0;

  for(int y = 0; y < 3; y++) {
    for(int x = 0; x < 3; x++) {
      if(x == 1 && y == 1) continue;
      if(h+y-1 < 0 || h+y-1 >= height) {
        continue;
      }
      if(w+x-1 < 0 || w+x-1 >= width) {
        continue;
      }
      if(cell[h+y-1][w+x-1] == 1) {
        cnt++;
      }
    }
  }

  return cnt;
}

void my_update_cells(const int height, const int width, int cell[height][width]) {
  //一旦移す
  int tempCell[height][width];
  for(int y = 0 ; y < height ; y++){
    for(int x = 0 ; x < width ; x++){
      tempCell[y][x] = cell[y][x];
    }
  }

  //更新処理
  for(int y = 0 ; y < height ; y++){
    for(int x = 0 ; x < width ; x++){
      int adjacent = my_count_adjacent_cells(y, x, height, width, cell);
      if(cell[y][x] == 1) {
        if(adjacent != 2 && adjacent != 3) {
          tempCell[y][x] = 0;
        }
      }
      if(cell[y][x] == 0) {
        if(adjacent == 3) {
          tempCell[y][x] = 1;
        }
      }
    }
  }

  //cellに値を移す
  for(int y = 0 ; y < height ; y++){
    for(int x = 0 ; x < width ; x++){
      cell[y][x] = tempCell[y][x];
    }
  }

}


int main(int argc, char **argv)
{
  FILE *fp = stdout;
  const int height = 40;
  const int width = 70;

  int cell[height][width];
  for(int y = 0 ; y < height ; y++){
    for(int x = 0 ; x < width ; x++){
      cell[y][x] = 0;
    }
  }

  /* ファイルを引数にとるか、ない場合はデフォルトの初期値を使う */
  if ( argc > 2 ) {
    fprintf(stderr, "usage: %s [filename for init]\n", argv[0]);
    return EXIT_FAILURE;
  }
  else if (argc == 2) {
    FILE *lgfile;
    if ( (lgfile = fopen(argv[1],"r")) != NULL ) {
      my_init_cells(height,width,cell,lgfile); // ファイルによる初期化
    }
    else{
      fprintf(stderr,"cannot open file %s\n",argv[1]);
      return EXIT_FAILURE;
    }
    fclose(lgfile);
  }
  else{
    my_init_cells(height, width, cell, NULL); // デフォルトの初期値を使う
  }

  my_print_cells(fp, 0, height, width, cell); // 表示する
  sleep(1); // 1秒休止

  /* 世代を進める*/
  for (int gen = 1 ;; gen++) {
    my_update_cells(height, width, cell); // セルを更新
    my_print_cells(fp, gen, height, width, cell);  // 表示する
    sleep(1); //1秒休止する
    fprintf(fp,"\e[%dA",height+3);//height+3 の分、カーソルを上に戻す(壁2、表示部1)
  }

  return EXIT_SUCCESS;
}
