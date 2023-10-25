#include <math.h>
#include <vplot.h>
#include <seplib.h>
#define NPMAX 32000
#define SCREENHT STANDARD_HEIGHT	/* STANDARD_HEIGHT is defined in
					 * vplot.h */
#define SCREENWD SCREENHT/SCREEN_RATIO	/* SCREEN_RATIO is defined in vplot.h */


 /*
  * datainfo contains the information normally associated with seplib
  * programs. 
  */
struct datainfo
{
    int             n1[NPMAX+1];
    int             n2;
    int             n3;
    float           d1[NPMAX+1];
    float           o1[NPMAX+1];
    float           d2;
    float           o2;
    float           d3;
    float           o3;
    int             esize;
};
 /*
  * coordinfo defines the users coordinates four corners.  The min's and
  * max's of the data 
  */
struct coordinfo
{
    float           min1;
    float           max1;
    float           min2;
    float           max2;
    int             transp;
    int             xreverse;
    int             yreverse;
    int		    labelrot;
    float	            fmin1;
    float	            fmin2;
    float	            fmax1;
    float	            fmax2;
    int             pad;		
    int             npad;		
};
 /*
  * axisinfo  defines what axis is.  Axisinfo is a general structure. an axis
  * is defined by it's origin,  where it is positioned, it's labeling
  * interval, its tic interval, its beginning value for labeling and tics,
  * it's label size labelfat and label position. 
  */
struct axisinfo
{
    float           labelsz;
    int             labelfat;
    int             fat[NPMAX+1];
    int             col[NPMAX+1];
    float           axisor;
    float           inch;
    int             wantaxis;
    char           wheretics[10];
    char           wherelabel[10];
    float           dnum;
    float           dtic;
    float           num0;
    float           tic0;
    int             ntic;
    char           label[280];
};
 /*
  * plotposition  contains all the information about the size and positioning
  * of the plot on the output device 
  */
struct plotposition
{
    float           inch1;
    float           inch2;
    float           xll;
    float           yll;
    float           xur;
    float           yur;
    float           screenht;
    float           screenwd;
    float           screenratio;
};
 /*
  * plotinfo contains the information about how the data will be plotted.
  * Type of line or symbol, size of symbol or thickness of line and color of
  * both symbol and line. 
  */
struct plotinfo
{
    int             lineunder;
    char            symbol[NPMAX+1];
    int             symbolsz[NPMAX+1];
    int             col[NPMAX+1];
    int             fat[NPMAX+1];
};
struct dashinfo
{
    float             dashtype[NPMAX+1];
    float           dash[2];
    float           gap[2];
};
 /*
  * gridinfo contains the information about plotting a grid on the data. the
  * intervals of the grid lines. and which axes to put the grid. 
  */
struct gridinfo
{
    int           grid;
    int           grid1;
    int           grid2;
    int           col[NPMAX+1]; 
    int           fat; 
    float         g1num;
    float         g2num;
};
 /*
  * titleinfo   contains information about the plot's title.  the size of the
  * title, fatness of title and which axis it will appear on. 
  */
struct titleinfo
{
    int             titlefat;
    float           titlesz;
    char           title[280];
    char           wheretitle[10];
    int 	   wanttitle;

};
 /*
  * colorinfo contains information about background  and fill colors of the
  * plot. 
  */
struct colorinfo
{
    float           backcol[3];
    float           fillcol[3];
};

#if defined(DEC3100) || defined(__stdc__ ) || defined(__STDC__) || defined( __cplusplus)

#define USE_PROTO

/* use prototyped function definitions for ansi-C and C++ */
int gl_optimal_scale(int n, float min, float max,
                         /*@out@*/ float *onum,float *dnum);
float gl_nice_number (float d);
float gl_power (float f, int ie);
extern gl_arrow(float,float,float,float,float);
extern gl_axisint(struct axisinfo* , struct axisinfo* , struct coordinfo*,struct plotposition*);
extern gl_barint (struct plotposition *, struct axisinfo *, struct plotposition *, struct axisinfo *, float *, float *, char *, int *, int,int);
extern gl_barplot (struct plotposition*, struct axisinfo*, float *, float *, char *, int, int);
extern gl_clip(float,float,float,float);
extern gl_color(int); 
extern gl_colorint(struct colorinfo*);
extern gl_coordinfo(struct plotposition*);
extern gl_coordint(struct plotposition *,struct coordinfo *,struct axisinfo *,struct axisinfo *);
extern gl_dash(struct dashinfo*);
extern gl_dashfig (struct dashinfo*, int);
extern gl_draw(float, float);
extern gl_erase();
extern gl_fat(int);
extern gl_fillin (struct coordinfo*, struct colorinfo*);
extern gl_framenum (int,float,float,float,float,float);
extern gl_getscl( struct coordinfo*, struct axisinfo* );
extern gl_gridint(struct gridinfo*, struct coordinfo*, struct axisinfo* , struct axisinfo* );
extern gl_gtext(float,float,float,float,float,float,char*,char*);
extern gl_invmassage(float*,float*,float,float);
extern gl_labelaxis(struct coordinfo*, struct axisinfo* );
extern gl_labeltic(struct coordinfo*, struct axisinfo*  );
extern gl_massage(float*,float*,float*,float*);
extern gl_minmax(struct coordinfo*);
extern gl_move(float, float); 
extern gl_opttic(float , float , float , float , float *, float);
extern gl_padint(struct coordinfo*);
extern gl_penup();
extern gl_plotaxis(struct axisinfo* , struct coordinfo*,int);
extern gl_plotframe(struct coordinfo*,int);
extern gl_plotgrid (struct coordinfo*,struct axisinfo*,struct gridinfo*,int );
extern gl_plotint( struct plotinfo *, struct dashinfo *);
extern gl_plotpram(struct colorinfo*, struct coordinfo*);
extern gl_plottic(struct coordinfo*, struct axisinfo* , int );
extern gl_plottitle (struct coordinfo*, struct titleinfo*, struct axisinfo*, int );
extern gl_purge();
extern gl_rotate(float*, float, float, struct datainfo*);
extern gl_rotate1(float*, float, float);
extern gl_simpleaxis( float,float,float,float,float,float, 
  float, float , float , char* , int,int,int,int,float,float );
extern gl_stdplot(struct datainfo *,struct coordinfo *,struct axisinfo *,struct axisinfo *,struct gridinfo *,struct titleinfo *,int,int,int,int);
extern gl_tfont(int,int,int);
extern gl_titleint();
extern gl_tjust(char*);
extern gl_transp( float*, float*,struct datainfo *); 
extern gl_uarea(float*,float*,int,int,int,int,int);
extern gl_udraw(float,float);
extern gl_uarrow(float,float,float,float,float);
extern gl_uclip(float,float,float,float);
extern gl_udraw(float,float);
extern gl_umove(float,float);
extern gl_upendn(float,float);
extern gl_upmark(int,int,int,float,float);
extern gl_vplotint(struct plotposition*, struct coordinfo*,struct axisinfo*,struct axisinfo* );
extern gl_where(float*,float*);
#endif
