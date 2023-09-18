#include<stdio.h>


typedef struct
{
    int order; //通道块在路径上的"序号" 
    int x; //通道块在迷宫中的"坐标位置" 
    int y;
    int direction; //从此通道块走向下一通道块的"方向" 
}SElemType; //栈的元素类型 



Status MazePath(MazeType maze, PosType start, PosType end)
{
    /若迷宫maze中存在从人口start到出口end的通道,则求得一条存放在栈中(从栈底到栈/顶),并返回 TRUE;否则返回 FALSE
    InitStack(S);
    curpos=start; //设定"当前位置"为"人口位置" 
    curstep =1; / 探索第一步 
    do {
        if(Pass(curpos)){/ 当前位置可以通过,即是未曾走到过的通道块
        FootPrint(curpos); /留下足迹 
        e=(curstep，curpos，1);
        Push(S,e); / 加人路径 
        if(curpos==end)return(TRUE);/到达终点(出口)
        curpos=NextPos(curpos，1); / 下一位置是当前位置的东邻 
        curstep++: / 探索下一步