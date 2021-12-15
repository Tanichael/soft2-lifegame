#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // sleep()関数を使う
#include <string.h>
#include <time.h>
#include <math.h>

//------------------------------------------------------------
//
// lifファイルの数字は 列 行 の順番
// 最初の生きたセルの割合がおよそ10%
// 存在するセルの比率を表示するようにする。
// 
// 初期のセルの数を270 ~ 289個に限定する
//
//------------------------------------------------------------

const int INIT_NUM_BASE = 270;
const int INIT_NUM_DIV = 20;



int my_init_cells(const int height, const int width, int cell[height][width], FILE* fp) {
  if(fp == NULL) {
    //初期のセルの比率を決定する
    srand((unsigned int)time(NULL));
    int init_num = rand() % 20 + 270;
    srand((unsigned int)time(NULL));
    for(int i = 0; i < init_num; i++) {
      int x;
      int y;
      x = rand() % width;
      y = rand() % height;
      cell[y][x] = 1;
    }
    

    return init_num; //セルの比率を返す
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
    int init_num = 0;

    char type[20];

    fgets(buf, bufsize, fp); //最初の行を読む
    strncpy(type, buf+strlen(buf)-5, 3);

    if(strcmp(type, "rle") == 0) {
      //RLEフォーマットの時
      int objX;
      int objY;
      char sobjX[20];
      char sobjY[20];
      int firstCommapos;
      int secondCommapos;
      int firstpos; //最初の数字の開始位置
      int secondpos; //二番目の数字の開始位置
      int numFlag = 0;
      int commaFlag = 0;

      do {
        fgets(buf, bufsize, fp);
      } while(buf[0] == '#');

      int len = strlen(buf);
      for(int i = 0; i < len; i++) {
        if(buf[i] == '=') {
          if(numFlag == 0) {
            firstpos = i+2;
            numFlag++;
          } else {
            secondpos = i+2;
          }
        }
        if(buf[i] == ',') {
          if(commaFlag == 0) {
            firstCommapos = i;
            commaFlag++;
          } else {
            secondCommapos = i;
            break;
          }
        }
      }

      strncpy(sobjX, buf+firstpos, firstCommapos-firstpos);
      strncpy(sobjY, buf+secondpos, secondCommapos-secondpos);

      objX = atoi(sobjX);
      objY = atoi(sobjY);

      x = 0;
      y = 0;
      int tempCnt = 1; //数字の文字数
      int tempCntPos = 0;
      int pos = 0;
      char data[3000];
      char sNum[20];
      int num;
      
      fgets(data, 3000, fp);
      printf("%s\n", data);

      for(int pos = 0; pos < strlen(data); pos++) {
        char c = data[pos];

        if(c == 'b' || c == 'o') {
          tempCnt = pos - tempCntPos;
          if(tempCnt == 0) {
            num = 1;
          } else {
            strncpy(sNum, data+tempCntPos, tempCnt);
            num = atoi(sNum);
          }

          if(c == 'o') {
            init_num += num;
          }

          for(int i = 0; i < num; i++) {
            if(c == 'b') {
              cell[y][x] = 0;
            } else if(c == 'o') {
              cell[y][x] = 1;
            }
            x++;
          }
          tempCntPos = pos + 1;
        } else if(c == '$' || c == '!') {
          if(x < objX) {
            for(int i = x; i < objX; i++) {
              cell[y][x] = 0;
              x++;
            }
          }
          x = 0;
          y++;
          tempCntPos = pos + 1;
          if(c == '!') {
            break;
          }
        }
      }

      return init_num;

    } else if(strcmp(buf, "#Life 1.06\n") == 0) {
      
      //Life 1.06形式の時
      while((c = fgetc(fp)) != EOF) {
       if(c == ' ') { //半角スペースの時
          //xを確定させる
          x = atoi(s);
          memset(s, '\0', sizeof(s));
          cnt = 0;
       } else if(c == '\n') { //改行の時
        //yを確定させる
          y = atoi(s);
          memset(s, '\0', sizeof(s));
          cnt = 0;
          cell[y][x] = 1;
          init_num++;
       } else if(c >= '0' && c <= '9'){ //数値の時
          s[cnt] = c;
          cnt++;
        }
      }
      return init_num;
    } else {
      return init_num;
    }
    
  }
}

void my_print_cells(FILE* fp, int gen, int init_num, const int height, const int width, int cell[height][width]) {
  float init_per = (float) init_num / (float)(height * width) * 100;
  fprintf(fp, "generation = %d, percentage = %3.2f\n", gen, init_per);
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

int my_update_cells(const int height, const int width, int cell[height][width]) {
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

  int cellNum = 0;
  //cellに値を移す
  for(int y = 0 ; y < height ; y++){
    for(int x = 0 ; x < width ; x++){
      if(tempCell[y][x] == 1) {
        cellNum++;
      }
      cell[y][x] = tempCell[y][x];
    }
  }

  return cellNum;
}


int main(int argc, char **argv)
{
  FILE *fp = stdout;
  const int height = 40;
  const int width = 70;

  int init_num;

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
      init_num = my_init_cells(height,width,cell,lgfile); // ファイルによる初期化
    }
    else{
      fprintf(stderr,"cannot open file %s\n",argv[1]);
      return EXIT_FAILURE;
    }
    fclose(lgfile);
  }
  else{
    init_num = my_init_cells(height, width, cell, NULL); // デフォルトの初期値を使う
  }

  my_print_cells(fp, 0, init_num, height, width, cell); // 表示する
  sleep(1); // 1秒休止

  int cell_num;
  /* 世代を進める*/
  for (int gen = 1 ;; gen++) {
    cell_num = my_update_cells(height, width, cell); // セルを更新
    my_print_cells(fp, gen, cell_num, height, width, cell);  // 表示する
    sleep(1); //1秒休止する
    fprintf(fp,"\e[%dA",height+3);//height+3 の分、カーソルを上に戻す(壁2、表示部1)
  }

  return EXIT_SUCCESS;
}
