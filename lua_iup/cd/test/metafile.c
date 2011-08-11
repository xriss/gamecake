#include <cd.h>
#include <cdmf.h>
void draw();
int marktype;


void main(void)
{
	cdCanvas *canvas;
	canvas = cdCreateCanvas(CD_METAFILE,"TESTE.MF 100x100");
	cdActivate(canvas);
	draw();
	cdKillCanvas(canvas);
}


void draw(void)
{
	cdMarkSize(5)
	cdMarkType(CD_PLUS);
	cdMark(10,90);
	cdMarkType(CD_STAR);
	cdMark(20,90);
	cdMarkType(CD_CIRCLE);
	cdMark(30,90);
	cdMarkType(CD_X);
	cdMark(40,90);
	cdMarkType(CD_BOX);
	cdMark(50,90);
	cdMarkType(CD_DIAMOND);
	cdMark(60,90);
	cdMarkType(CD_HOLLOW_CIRCLE);
	cdMark(70,90);
	cdMarkType(CD_HOLLOW_BOX);
	cdMark(80,90);
	cdMarkType(CD_HOLLOW_DIAMOND);
	cdMark(90,90);

	cdLineStyle(CD_CONTINUOUS);
	cdLine(10,80,80,80);
	cdLineStyle(CD_DASHED);
	cdLine(10,75,80,75);
	cdLineStyle(CD_DOTTED);
	cdLine(10,70,80,70);
	cdLineStyle(CD_DASH_DOT);
	cdLine(10,65,80,65);
	cdLineStyle(CD_DASH_DOT_DOT);
	cdLine(10,60,80,60);

	cdLineStyle(CD_CONTINUOUS);

	cdHatch(CD_HORIZONTAL);

	cdBegin(CD_FILL);
	cdVertex(10,50);
	cdVertex(50,50);
	cdVertex(50,10);
	cdVertex(10,10);
	cdEnd();

	cdHatch(CD_VERTICAL);
	cdBegin(CD_FILL);
	cdVertex(60,50);
	cdVertex(100,50);
	cdVertex(100,10);
	cdVertex(60,10);
	cdEnd();	

	cdHatch(CD_FDIAGONAL);
	cdBegin(CD_FILL);
	cdVertex(110,50);
	cdVertex(150,50);
	cdVertex(150,10);
	cdVertex(110,10);
	cdEnd();	

	cdHatch(CD_BDIAGONAL);
	cdBegin(CD_FILL);
	cdVertex(160,50);
	cdVertex(200,50);
	cdVertex(200,10);
	cdVertex(160,10);
	cdEnd();	

	cdHatch(CD_CROSS);
	cdBegin(CD_FILL);
	cdVertex(210,50);
	cdVertex(250,50);
	cdVertex(250,10);
	cdVertex(210,10);
	cdEnd();	

	cdHatch(CD_DIAGCROSS);
	cdBegin(CD_FILL);
	cdVertex(260,50);
	cdVertex(300,50);
	cdVertex(300,10);
	cdVertex(260,10);
	cdEnd();	
	
	cdFont(CD_SYSTEM,CD_BOLD,CD_STANDARD);
	cdText(10,100,'Teste');
	cdFont(CD_COURIER,CD_BOLD,CD_STANDARD);
	cdText(60,100,'Teste');
	cdFont(CD_TIMES_ROMAN,CD_BOLD,CD_STANDARD);
	cdText(110,100,'Teste');
	cdFont(CD_HELVETICA,CD_BOLD,CD_STANDARD);
	cdText(160,100,'Teste');
}
