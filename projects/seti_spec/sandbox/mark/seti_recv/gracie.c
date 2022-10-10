#include <stdio.h>
#include <stdlib.h>
#include <grace_np.h>

void my_error_function(const char *msg)
{
    fprintf(stderr, "library message: \"%s\"\n", msg);
}

void grace_open(int bufsize)
{
    GraceRegisterErrorFunction(my_error_function);

    if(GraceOpenVA("xmgrace", bufsize, "-free","-nosafe", "-noask", NULL))
    {
        fprintf(stderr, "Can't run Grace. \n");
        exit(EXIT_FAILURE);
    }

}

void grace_init(int maxbin,float maxvalue,int xmax)
{

//fonts
    GracePrintf("map font 0 to \"Times-Roman\", \"Times-Roman\"");
    GracePrintf("map font 1 to \"Times-Italic\", \"Times-Italic\"");
    GracePrintf("map font 2 to \"Times-Bold\", \"Times-Bold\"");
    GracePrintf("map font 3 to \"Times-BoldItalic\", \"Times-BoldItalic\"");
    GracePrintf("map font 4 to \"Helvetica\", \"Helvetica\"");
    GracePrintf("map font 5 to \"Helvetica-Oblique\", \"Helvetica-Oblique\"");
    GracePrintf("map font 6 to \"Helvetica-Bold\", \"Helvetica-Bold\"");
    GracePrintf("map font 7 to \"Helvetica-BoldOblique\", \"Helvetica-BoldOblique\"");
    GracePrintf("map font 8 to \"Courier\", \"Courier\"");
    GracePrintf("map font 9 to \"Courier-Oblique\", \"Courier-Oblique\"");
    GracePrintf("map font 10 to \"Courier-Bold\", \"Courier-Bold\"");
    GracePrintf("map font 11 to \"Courier-BoldOblique\", \"Courier-BoldOblique\"");
    GracePrintf("map font 12 to \"Symbol\", \"Symbol\"");
    GracePrintf("map font 13 to \"ZapfDingbats\", \"ZapfDingbats\"");

//colors

    GracePrintf("map color 0 to (255,255,255),\"white\" ");
    GracePrintf("map color 1 to (0,0,0),\"black\" ");
    GracePrintf("map color 2 to (255,0,0),\"red\" ");
    GracePrintf("map color 3 to (0, 255, 0), \"green\"");
    GracePrintf("map color 4 to (0, 0, 255), \"blue\"");
    GracePrintf("map color 5 to (255, 255, 0), \"yellow\"");
    GracePrintf("map color 6 to (188, 143, 143), \"brown\"");
    GracePrintf("map color 7 to (220, 220, 220), \"grey\"");
    GracePrintf("map color 8 to (148, 0, 211), \"violet\"");
    GracePrintf("map color 9 to (0, 255, 255), \"cyan\"");
    GracePrintf("map color 10 to (255, 0, 255), \"magenta\"");
    GracePrintf("map color 11 to (255, 165, 0), \"orange\"");
    GracePrintf("map color 12 to (114, 33, 188), \"indigo\"");
    GracePrintf("map color 13 to (103, 7, 72), \"maroon\"");
    GracePrintf("map color 14 to (64, 224, 208), \"turquoise\"");
    GracePrintf("map color 15 to (0, 139, 0), \"green4\"");

//defaults

    GracePrintf("default linewidth 1.0");
    GracePrintf("default linestyle 1");
    GracePrintf("default color 1");
    GracePrintf("default pattern 1");
    GracePrintf("default font 8");
    GracePrintf("default char size 1.000000");
    GracePrintf("default symbol size 1.000000");
    GracePrintf("default sformat \"%16.8g\"");

//background color

    GracePrintf("background color %d",7);

//g0

    GracePrintf("g0 on");
    GracePrintf("g0 hidden false");
    GracePrintf("g0 type XY");
    GracePrintf("g0 stacked false");
    GracePrintf("g0 bar hgap 0.000000");
    GracePrintf("g0 fixedpoint off");
    GracePrintf("g0 fixedpoint type 0");
    GracePrintf("g0 fixedpoint xy 0.000000, 0.000000");
    GracePrintf("g0 fixedpoint format general general");
    GracePrintf("g0 fixedpoint prec 6, 6");
    GracePrintf("with g0");

//g0 xaxis

    GracePrintf("world xmax %d",xmax);
    GracePrintf("world xmin %d",0);
    GracePrintf("world ymax %d",1000000000);
    GracePrintf("world ymin %d",1);
    GracePrintf("stack world 0, 0, 0, 0 tick 0, 0, 0, 0");
    GracePrintf("yaxes scale Logarithmic");
    GracePrintf("xaxis tick major 32000");
    GracePrintf("xaxis tick minor 100");

//g0 xaxis label 

    GracePrintf("xaxis label \"bins\"");
    GracePrintf("xaxis label font \"Courier\"");
    GracePrintf("xaxis label char size .7");
    GracePrintf("xaxis ticklabel font \"Courier\"");
    GracePrintf("xaxis ticklabel char size .7");
    GracePrintf("xaxis label color %d",1);

//g0 yaxis label

    GracePrintf("yaxis label \"amplitude\"");
    GracePrintf("yaxis label font \"Courier\""); 
    GracePrintf("yaxis label char size .7");
    GracePrintf("yaxis ticklabel font \"Courier\"");
    GracePrintf("yaxis ticklabel char size .7");
    GracePrintf("yaxis ticklabel format power");
    GracePrintf("yaxis tick minor ticks 5");
    GracePrintf("yaxis ticklabel prec 0");
    GracePrintf("yaxis label color %d",1);

//g0 title properties
    
    GracePrintf("title \"Seti Spectrometer\"");
    GracePrintf("title color %d",1);
    GracePrintf("title font \"Courier\"");
    GracePrintf("title size .9");
    GracePrintf("subtitle \"max value %f in bin %d \"",maxvalue,maxbin);
    GracePrintf("subtitle color %d",1);
    GracePrintf("subtitle font 8");
    GracePrintf("subtitle size .8");

//create set s0 and properties for graph g0

    GracePrintf("s0 on");
    GracePrintf("s0 hidden false");
    GracePrintf("s0 type xy");
    GracePrintf("s0 symbol 2");
    GracePrintf("s0 symbol color 1");
    GracePrintf("s0 symbol size .3");
    GracePrintf("s0 symbol char 65");
    GracePrintf("s0 symbol fill pattern 0");
    GracePrintf("s0 symbol fill color 0");
    GracePrintf("s0 line linestyle 0");
    GracePrintf("s0 line color 1");
    GracePrintf("s0 fill type 0");
    GracePrintf("s0 fill rule 5");
    GracePrintf("s0 fill color 0");
    GracePrintf("s0 fill pattern 1");
  
//create set s1 and properties for graph g0

    GracePrintf("s1 on");
    GracePrintf("s1 hidden false");
    GracePrintf("s1 type xy");
    GracePrintf("s1 symbol 2");
    GracePrintf("s1 symbol color 0");
    GracePrintf("s1 symbol size .3");
    GracePrintf("s1 symbol char 65");
    GracePrintf("s1 symbol fill pattern 0");
    GracePrintf("s1 symbol fill color 9");
    GracePrintf("s1 line linestyle 0");
    GracePrintf("s1 line color 1");
    GracePrintf("s1 fill type 0");
    GracePrintf("s1 fill rule 5");
    GracePrintf("s1 fill color 0");
    GracePrintf("s1 fill pattern 1");

//create set s2 and properties for graph g0

    GracePrintf("s2 on");
    GracePrintf("s2 hidden false");
    GracePrintf("s2 type xy");
    GracePrintf("s2 symbol 2");
    GracePrintf("s2 symbol color 14");
    GracePrintf("s2 symbol size .3");
    GracePrintf("s2 symbol char 65");
    GracePrintf("s2 symbol fill pattern 0");
    GracePrintf("s2 symbol fill color 9");
    GracePrintf("s2 line linestyle 0");
    GracePrintf("s2 line color 1");
    GracePrintf("s2 fill type 0");
    GracePrintf("s2 fill rule 5");
    GracePrintf("s2 fill color 0");
    GracePrintf("s2 fill pattern 1");

}








 
