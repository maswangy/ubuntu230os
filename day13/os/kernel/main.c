
/**********************************************************************
关于刷新图层加速的部分没有实现，感觉没有太大的必要去理解学习这一部分，不是重点
 **********************************************************************/
#include<header.h>
#include<fontascii.h>
#include<GUIType.h>
#include<GUI.h>

//需要用到的函数，halt,cli,out8,read_eflags,write_eflags,这些函数在x86.h中
//init _palette, set_palette 这两个函数我想放在screen.c中

#define black 0
#define red   1
#define green 2
extern  GUI_CONTEXT        GUI_Context;

void bootmain(void)
{

/*进入30os的部分*/
struct boot_info *bootp=(struct boot_info *)ADDR_BOOT;
init_screen((struct boot_info * )bootp);
init_palette();  //color table from 0 to 15
//clear_screen(8);   	//red
//draw_window();
int mx,my;//mouse position
//display mouse logo
char mousepic[16*16];     //mouse logo buffer

//display_mouse(bootp->vram,bootp->xsize,16,16,mx,my,mousepic,16);
cli();

//set gdt idt
init_gdtidt();
//remap irq 0-15
//函数中：　irq 1(keyboard)对应设置中断号int0x21,    irq　12(mouse)对应的中断号是int0x2c 要写中断服务程序了。
init_pic();

//设置完了gdt,idt后再enable cpu interrupt才是安全的

unsigned char s[40];		    //sprintf buffer
unsigned char keybuf[32];	    //keyfifo
unsigned char mousebuf[128];	//mousefifo
unsigned char data;		        //temp variable to get fifo data
int count=0;
fifo8_init(&keyfifo ,32,keybuf);//keyfifo是一个global data defined in int.c
fifo8_init(&mousefifo,128,mousebuf);

//enable timer ,keyboard and mouse   //1111 1000 后面的三个0代表 accept interrupt request, irq0=timer interrupt
outb(PIC0_IMR, 0xf8);//1111 1000  irq 1 2打开 因为keyboard是irq 1,irq2 enable 8259B 芯片发生的中断请求                                 // enable pic slave and keyboard interrupt
//enable mouse interrupt 1110 1111  irq 12 打开　mouse是irq 12  所以要把pic 1 pic 2的芯片中断响应位打开。
outb(PIC1_IMR, 0xef);
//初始化　鼠标按键控制电路
init_keyboard();

//enable cpu interrupt


struct MOUSE_DEC mdec;
enable_mouse(&mdec);   //这里会产生一个mouse interrupt

unsigned int memtotal;
//get the total memory
memtotal=memtest(0x400000,0xffffffff);
//mem=mem>>20; //convert to MBytes
//sprintf(s,"memory:%dMbytes",mem);
//puts8((char *)bootp->vram ,bootp->xsize,0,100,0,s);
Memman * memman=(Memman *)0x3c0000;
memman_init(memman);
//memman_free(memman,0x1000,0x9e000);
memman_free(memman,0x400000,memtotal-0x400000);
//memman_free(memman,0x600000,0x400000);
//memman_free(memman,0xb00000,0x400000);
char *desktop=(unsigned char*)memman_alloc(memman,320*200);
printdebug(desktop,0);
//while(1);
char *win_buf=(unsigned char*)memman_alloc_4K(memman,160*65);
TIMERCTL *timerctl=(TIMERCTL *)memman_alloc_4K(memman,sizeof(TIMERCTL));
init_pit(timerctl);//init timerctl
sti();


// try some function in ucgui
GUI_Init();
draw_win_buf(desktop);

GUI_Context.TextMode=0 ;
GUI_SetBkColorIndex(0);
GUI_SetColorIndex(1);
GUI_GotoXY(200,0);
/*
GUI_RECT rClient;
rClient.x0=200;
rClient.x1=240;
rClient.y0=0;
rClient.y1=8;
GUI_DispStringInRect("abccc", &rClient, GUI_TA_HCENTER | GUI_TA_VCENTER);
*/

//GUI_Clear(); //will clear the screen use bkcolor=0 now black
LCD_L0_DrawHLine  (0, 100,  319);
GUI_FillCircle(50,50,30);
//GUI_ClearRect(50,50,200,200);

GUI_DispStringAt("hello",250,100);
GUI_GotoXY(100,10);

GUI_DispStringLen("abcdefg",10);
GUI_DispChars('a',10);
//GUI_DispStringAtCEOL("clear line",100,10);
//GUI_DispStringAt("great this is",240,108);
GUI_DispString("\ngreat this is");
//GUI_GotoXY(200,0);
//GUI_Context.pAFont->pfDispChar('b');
GUI_DispDec(33,8);
GUI_DispDecMin(-1234);
GUI_Context.DispPosX+=10;
GUI_DispSDec(32,3);
GUI_Context.DispPosX+=2;
GUI_DispFloatMin(3.124,2);
GUI_Context.DispPosX+=2;
GUI_DispSFloatFix(4.52432,9,5);
GUI_GotoXY(100,30);
GUI_DispString("15 binary:");
GUI_DispBin(15,4);
GUI_Context.DispPosX+=2;
GUI_DispString("15 hex:");
GUI_DispHex(15,4);
GUI_SetPenSize(5);
GUI_DrawPoint(199,180);
GUI_FillRect(0,0,20,20);
GUI_InvertRect(0,0,20,20);
GUI_ClearRect(0,0,20,20);

GUI_DrawHLine(120,0,320);

GUI_SetDrawMode(GUI_DRAWMODE_NORMAL);
GUI_FillCircle(120, 64, 40);
GUI_SetDrawMode(GUI_DRAWMODE_XOR);
GUI_FillCircle(140, 84, 40);

/*
static const GUI_POINT aPointArrow[] = {
{ 0, -5},
{-40, -35},
{-10, -25},
{-10, -85},
{ 10, -85},
{ 10, -25},
{ 40, -35},
};
int Cnt =0;
GUI_SetBkColorIndex(7);
GUI_SetColorIndex(0);
GUI_DispStringAt("Polygons of arbitrary shape", 0, 0);
GUI_DispStringAt("in any color", 120, 20);
GUI_SetColor(GUI_BLUE);
// 画一个填充多边形
GUI_FillPolygon (&aPointArrow[0],7,150,150);
*/

unsigned i;
for (i=10; i<50; i++)
GUI_DrawCircle(60,60,i);

make_window8(win_buf,160,68,"timer");
init_mouse(mousepic,99);	//99　means background color


sprintf(s,"memory:%dMB,free:%dMB,%d",memtotal>>20
,memman_avail(memman)>>20,memman->cellnum);
puts8(desktop ,bootp->xsize,0,150,0,s);

SHTCTL *shtctl;
shtctl=shtctl_init(memman,bootp->vram,bootp->xsize,bootp->ysize);

SHEET *sht_back,*sht_mouse,*sht_win;
//allocate a sheet space from shtctl
sht_back=sheet_alloc(shtctl);
sht_mouse=sheet_alloc(shtctl);
sht_win=sheet_alloc(shtctl);

//write something inside the window
//puts8(win_buf ,160,24,28,0,"hello ,easy os");
//puts8(win_buf ,160,24,44,0,"second line");//y=28+16=44

//hoop the buffer with sheet
sheet_setbuf(sht_back,desktop,320,200,-1);
sheet_setbuf(sht_mouse,mousepic,16,16,99);
sheet_setbuf(sht_win,win_buf,160,65,-1);
mx=0;my=0;//set mouse initial position
sheet_move(sht_back,0,0);
sheet_move(sht_mouse,mx,my);
sheet_move(sht_win,80,72);

//set sht_back to layer0 ;set sht_mouse to layer1
sheet_updown(sht_back,0);
sheet_updown(sht_win,1);
sheet_updown(sht_mouse,2);
//refresh a specific rectangle
sheet_refresh(sht_back,0,0,bootp->xsize,bootp->ysize);

struct FIFO8 timerfifo;
char timerbuf[8];
TIMER *timer,*timer2,*timer3;

//just one fifio
fifo8_init(&timerfifo,8,timerbuf);

timer=timer_alloc(timerctl,0);
timer2=timer_alloc(timerctl,1);
timer3=timer_alloc(timerctl,2);

timer_init(timer,&timerfifo,10);//10s
timer_init(timer2,&timerfifo,3);//3s
timer_init(timer3,&timerfifo,1);//50ms
//while(1);
timer_settime(timer,1000,timerctl);
timer_settime(timer2,300,timerctl);
timer_settime(timer3,50,timerctl);

while(1)
 {

    //wrtrfrsh(sht_win,24,28,0,7,"count",5);
    sprintf(s,"%d ",timerctl->count);
    wrtrfrsh(sht_win,20,28,0,7,s,7);
    GUI_GotoXY(280,180);
    GUI_DispDec(timerctl->count,5);
    sheet_refresh(sht_back,280,180,320,190);
    cli();//disable cpu interrupt
     // sti();
    //while(1);
   if(fifo8_status(&keyfifo) +
   fifo8_status(&mousefifo)  +
   fifo8_status(&timerfifo)
    == 0)//no data in keyfifo and mousefifo
    {
    sti();
    //hlt();//wait for interrupt
   }
   else
   {
      if(fifo8_status(&keyfifo) != 0)
      {
        data=fifo8_read(&keyfifo);
        sti();
      }//end of keyboard decoder
      else if(fifo8_status(&mousefifo) != 0)//we have mouse interrupt data to process
      {
        data=fifo8_read(&mousefifo);
        sti();
        if(mouse_decode(&mdec,data))
        {
              //3个字节都得到了
              switch (mdec.button)
              {
                case 1:s[1]='L';break;
                case 2:s[3]='R';break;
                case 4:s[2]='M';break;
              }
              sprintf(s,"[lmr:%d %d]",mdec.x,mdec.y);
              boxfill8(desktop,320,0,32,16,32+15*8-1,33);//一个黑色的小box
              puts8(desktop,bootp->xsize,32,16,7,s);     //display e0
              sheet_refresh(sht_back,32,16,32+20*8-1,31);
        #define white 7
               //because we use sheet layer ,so we do not need this any more
              //boxfill8(p,320,white,mx,my,mx+15,my+15);//用背景色把鼠标原来的color填充，这样不会看到鼠标移动的path
              mx +=mdec.x;//mx=mx+dx
              my +=mdec.y;//my=my+dy
              if(mx<0)
              {
                mx=0;
              }
              if(my<0)
              {
                my=0;
              }


              if(mx>bootp->xsize-1)
              {
                mx=bootp->xsize-1;
              }

              if(my>bootp->ysize-1)
              {
                my=bootp->ysize-1;
              }
              sprintf(s,"(%d, %d)",mx,my);
              boxfill8(desktop,320,0,0,0,79,15);//坐标的背景色
              puts8(desktop ,bootp->xsize,0,0,7,s);//显示坐标
              sheet_refresh(sht_back,0,0,bootp->xsize,15);
              sheet_move(sht_mouse,mx,my);
        }
      }//end of mouse decoder
      else if(fifo8_status(&timerfifo)!=0)
      {
         sti();
         data=fifo8_read(&timerfifo);
         if(data==10)
         {
            puts8(desktop ,bootp->xsize,0,64,0,"10[sec]");//显示坐标
            sheet_refresh(sht_back,0,64,bootp->xsize,80);
         }
         if(data==3)
         {
            puts8(desktop ,bootp->xsize,0,80,0,"3[sec]");//显示坐标
            sheet_refresh(sht_back,0,80,bootp->xsize,96);
         }
         else
         {
             if(data!=0)
             {
                    timer_init(timer3,&timerfifo,0);//50ms
                    boxfill8(desktop,bootp->xsize,7,8,96,15,111);
                    GUI_SetBkColorIndex(0);
                    GUI_SetColorIndex(6);
                    GUI_DispStringAt("hello wolrd",250,100);
             }
             else
             {

                    timer_init(timer3,&timerfifo,1);//50ms
                    boxfill8(desktop,bootp->xsize,0,8,96,15,111);
                    GUI_SetBkColorIndex(6);
                    GUI_SetColorIndex(0);
                    GUI_DispStringAt("hello",250,100);
             }
                timer_settime(timer3,50,timerctl);
                sheet_refresh(sht_back,8,96,15,111);
                sheet_refresh(sht_back,250,100,320,108);
         }
      }//end of timer3

   }

 }
}
















