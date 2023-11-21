/*
 * numworks-julia - A Julia and Mandelbrot fractal generator
 * 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 */

#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <complex.h>
#include <math.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Julia";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

#define SQRT2 1.4142135623730951

#define TILESIZE 8

#define SCREEN_WIDTH EADK_SCREEN_WIDTH
#define SCREEN_HEIGHT 200
#define YOFF 18

#define NX (SCREEN_WIDTH/TILESIZE)
#define NY (SCREEN_HEIGHT/TILESIZE)
#define SCREEN_RATIO (SCREEN_HEIGHT/(float) SCREEN_WIDTH)

#define SIGN(x) (((x) < 0) ? (-1) : (1))
#define CSIGN(x) (((x) < 0) ? ("-") : (" "))

struct viewport 
{
  float xc,yc,width;
};

const struct viewport defaultview={.xc=0,.yc=0,.width=4};

struct cursor 
{
  int x,y;
  eadk_color_t orgbuf[5*5];
  eadk_color_t buf[5*5];
};

typedef enum {NONE, LEFT, RIGHT, UP, DOWN, REFRESH} update_mode;

struct tile_update
{
  int x,y, nx,ny;
};

void statuslinemsg(const char * msg) {
  uint16_t c=0xfda6;
  eadk_display_push_rect_uniform((eadk_rect_t){0,0,320,18}, c);
  eadk_display_draw_string(msg, (eadk_point_t){160-5*strlen(msg), 0}, true, eadk_color_white, c);
}

void bottommsg(const char * msg)
{
}

void move_fb(struct viewport *vw, update_mode mode)
{
  eadk_color_t tile[TILESIZE*TILESIZE];
  int dx,dy,xs,xe,ys,ye;

  float tw=TILESIZE*vw->width/SCREEN_WIDTH;

  switch(mode)
  {
    case LEFT:
      vw->xc-=tw;
      dx=1;dy=0;
      break;
    case RIGHT:
      vw->xc+=tw;
      dx=-1;dy=0;
      break;
    case UP:
      vw->yc+=tw;
      dx=0;dy=1;
      break;
    case DOWN:
      vw->yc-=tw;
      dx=0;dy=-1;
      break;

    default:
      return;
  }

  xs=0;xe=NX; if(dx==1) {xs=NX-1;xe=-1;} 
  ys=0;ye=NY; if(dy==1) {ys=NY-1;ye=-1;} 

  for(int tx=xs;tx!=xe;tx+=SIGN(xe-xs))
    for(int ty=ys;ty!=ye;ty+=SIGN(ye-ys))
      if(tx+dx>=0 && tx+dx<NX && ty+dy>=0 && ty+dy<NY) 
      {
        eadk_display_pull_rect((eadk_rect_t){tx*TILESIZE,ty*TILESIZE+YOFF,TILESIZE,TILESIZE}, tile);
        eadk_display_push_rect((eadk_rect_t){(tx+dx)*TILESIZE,(ty+dy)*TILESIZE+YOFF,TILESIZE,TILESIZE}, tile);
      }
}

struct tile_update get_update_tiles(update_mode mode)
{
  switch(mode)
  {
    case LEFT:
      return (struct tile_update){0,0,1,NY};
    case RIGHT:
      return (struct tile_update){NX-1,0,1,NY};
    case UP:
      return (struct tile_update){0,0,NX,1};
    case DOWN:
      return (struct tile_update){0,NY-1,NX,1};
    default:
      return (struct tile_update){0,0,NX,NY};
  }
}

void julia(float complex c, int Niteration, struct viewport *vw, update_mode mode)
{
  eadk_color_t tile[TILESIZE*TILESIZE];
  struct tile_update tu=get_update_tiles(mode);
  float complex z;
  int i;
  
  float pw=vw->width/SCREEN_WIDTH;
  float bottom_x=vw->xc-vw->width/2;
  float top_y=vw->yc+SCREEN_RATIO*vw->width/2;
  for(int tx=tu.x;tx<tu.x+tu.nx;tx++)
    for(int ty=tu.y;ty<tu.y+tu.ny;ty++)
      {
        for(int ix=0;ix<TILESIZE;ix++)
          for(int iy=0;iy<TILESIZE;iy++)
          {
            z=(bottom_x+pw*(ix+tx*TILESIZE))+((top_y+(-pw)*(iy+ty*TILESIZE)))*I;
            i=0;
            while(i<Niteration && cimagf(z)*cimagf(z)+crealf(z)*crealf(z)<4) 
            {
              i++;
              z=z*z+c;
            }
            tile[ix+TILESIZE*iy]=(eadk_color_t) (((float)i/Niteration)*eadk_color_red);
          }
        eadk_display_push_rect((eadk_rect_t){tx*TILESIZE,ty*TILESIZE+YOFF,TILESIZE,TILESIZE}, tile);
      }
}

void mandelbrot(int Niteration, struct viewport *vw, update_mode mode)
{
  eadk_color_t tile[TILESIZE*TILESIZE];
  struct tile_update tu=get_update_tiles(mode);
  float complex z,c;
  int i;
  
  float pw=vw->width/SCREEN_WIDTH;
  float bottom_x=vw->xc-vw->width/2;
  float top_y=vw->yc+SCREEN_RATIO*vw->width/2;
  for(int tx=tu.x;tx<tu.x+tu.nx;tx++)
    for(int ty=tu.y;ty<tu.y+tu.ny;ty++)
      {
        for(int ix=0;ix<TILESIZE;ix++)
          for(int iy=0;iy<TILESIZE;iy++)
          {
            z=0;
            c=(bottom_x+pw*(ix+tx*TILESIZE))+((top_y+(-pw)*(iy+ty*TILESIZE)))*I;
            i=0;
            while(i<Niteration && cimagf(z)*cimagf(z)+crealf(z)*crealf(z)<4) 
            {
              i++;
              z=z*z+c;
            }
            tile[ix+TILESIZE*iy]=(eadk_color_t) (((float)i/Niteration)*eadk_color_red);
          }
        eadk_display_push_rect((eadk_rect_t){tx*TILESIZE,ty*TILESIZE+YOFF,TILESIZE,TILESIZE}, tile);
      }
}

void hide_cursor(struct cursor *c)
{
  eadk_display_push_rect((eadk_rect_t){c->x-2,c->y-2+YOFF,5,5}, c->orgbuf);
}
void show_cursor(struct cursor *c)
{
  eadk_display_pull_rect((eadk_rect_t){c->x-2,c->y-2+YOFF,5,5}, c->orgbuf);
  eadk_display_pull_rect((eadk_rect_t){c->x-2,c->y-2+YOFF,5,5}, c->buf);
  c->buf[2]=eadk_color_white;c->buf[7]=eadk_color_white;
  c->buf[10]=eadk_color_white;c->buf[11]=eadk_color_white;
  c->buf[12]=eadk_color_white;
  c->buf[13]=eadk_color_white;c->buf[14]=eadk_color_white;
  c->buf[17]=eadk_color_white;c->buf[22]=eadk_color_white;
  eadk_display_push_rect((eadk_rect_t){c->x-2,c->y-2+YOFF,5,5}, c->buf);

}

float complex c_from_cursor(struct cursor *c, struct viewport *vw)
{
  return (vw->xc-vw->width/2+(vw->width*c->x)/SCREEN_WIDTH)+
          (vw->yc+SCREEN_RATIO*vw->width/2-(SCREEN_RATIO*vw->width*c->y)/SCREEN_HEIGHT)*I;
}

void mygcvt(float x, int ndig, char * buf)
{
  float _x, f, i;
  _x=x*SIGN(x);
  f=modff(_x,&i);
  sprintf(buf,"%s%1d.%04d", CSIGN(x), (int) i, (int) (10000*f));
} // so we can use nano.spec

void show_status(float complex c, int Niterations)
{
  char msg[128], buf1[8],buf2[8];
  float cr=crealf(c), ci=cimagf(c);
  eadk_display_push_rect_uniform((eadk_rect_t){0,220,EADK_SCREEN_WIDTH,20}, eadk_color_black);  
  mygcvt(cr,4,buf1);
  mygcvt(ci,4,buf2);
  sprintf(msg, "c=%s+%sI,  Nit=%4d", buf1, buf2, Niterations);
  eadk_display_draw_string(msg, (eadk_point_t){160-(int)(3.5*strlen(msg)), 222}, false, 0xfda6, eadk_color_black);
  
}

void mainloop() {
  int Niterations=100;
  struct viewport vw=defaultview, jvw;
  struct cursor c={.x=SCREEN_WIDTH/2 ,.y=SCREEN_HEIGHT/2, .buf={0}};
  update_mode mode;
  statuslinemsg("Julia Explorer");
  bool show_julia=false;
  float complex cconst=c_from_cursor(&c, &vw);

  mandelbrot(Niterations,&vw,REFRESH);
  show_cursor(&c);
  cconst=c_from_cursor(&c, &vw);
  show_status(cconst, Niterations);

  while (true) 
  {
    int32_t timeout = 1000;
    eadk_event_t ev = eadk_event_get(&timeout); 
    mode=NONE;
    switch (ev)
    {
      case eadk_key_on_off:
        return;
      case eadk_event_left:
        mode=LEFT;
        break;
      case eadk_event_right:
        mode=RIGHT;
        break;
      case eadk_event_up:
        mode=UP;
        break;
      case eadk_event_down:
        mode=DOWN;
        break;
      case eadk_event_plus:
        vw.width/=SQRT2;
        mode=REFRESH;
        break;
      case eadk_event_minus:
        vw.width*=SQRT2;
        mode=REFRESH;
        break;
      case eadk_event_multiplication:
        Niterations*=1.2;
        if(Niterations>1000) Niterations=1000;
        mode=REFRESH;
        break;
      case eadk_event_division:
        Niterations/=1.2;
        if(Niterations<30) Niterations=30;
        mode=REFRESH;
        break;
      case eadk_event_ok:  
      case eadk_event_back:
        show_julia=!show_julia;
        jvw=defaultview;
        mode=REFRESH;
    }
    if(mode != NONE) 
    {
      hide_cursor(&c);
      move_fb(&vw,mode);

      cconst=c_from_cursor(&c, &vw);
      if(show_julia)
        julia(cconst,Niterations,&jvw, mode);
      else
        mandelbrot(Niterations,&vw,mode);
      show_cursor(&c);
      show_status(cconst, Niterations);
    }
    
  }
}

int main(int argc, char * argv[]) {
  eadk_display_push_rect_uniform((eadk_rect_t){0,0,EADK_SCREEN_WIDTH,EADK_SCREEN_HEIGHT}, 0x0);
  mainloop();
}
