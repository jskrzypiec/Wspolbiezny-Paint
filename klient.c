#define _REENTRANT

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//socket
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
//X11
#include <X11/Xlib.h>
#include <X11/X.h>

#include <arpa/inet.h> //INET_PTON


typedef struct{
	int userId;
	XImage *image;
} dane;

dane d1;

struct sockaddr_in recipient;
int socket_descriptor, size;

//X11
Display* mydisplay;
Window mywindow;
XSetWindowAttributes mywindowattributes;
XGCValues mygcvalues;
GC mygc1, mygc2;
Visual* myvisual;
int mydepth;
int myscreen;
Colormap mycolormap;
XColor mycolor1, mycolor2, mycolor3, mycolor4, dummy;
XEvent myevent;
//XImage *image;

void otworzOkno(){
	XInitThreads();

	mydisplay = XOpenDisplay(""); 
  myscreen = DefaultScreen(mydisplay);
  myvisual = DefaultVisual(mydisplay, myscreen);
  mydepth = DefaultDepth(mydisplay, myscreen);
  mywindowattributes.background_pixel = XWhitePixel(mydisplay, myscreen);
  mywindowattributes.override_redirect = True;

	mywindow = XCreateWindow(mydisplay,XRootWindow(mydisplay, myscreen),
			   100, 100, 1240, 874, 10, mydepth, InputOutput,
			   myvisual, CWBackPixel|CWOverrideRedirect,
			   &mywindowattributes);

	XSelectInput(mydisplay, mywindow, ExposureMask|KeyPressMask|ButtonPressMask|ButtonReleaseMask
|ButtonMotionMask);
	
	mycolormap = DefaultColormap(mydisplay, myscreen);

  XAllocNamedColor(mydisplay, mycolormap,"black", &mycolor1, &dummy);
  XAllocNamedColor(mydisplay, mycolormap,"white", &mycolor2, &dummy);
  XAllocNamedColor(mydisplay, mycolormap,"red", &mycolor3, &dummy);
	XAllocNamedColor(mydisplay, mycolormap,"blue", &mycolor4, &dummy);

	XStoreName(mydisplay,mywindow,"jsPAINT - aka paint by Jakub Skrzypiec @2020.05");

  XMapWindow(mydisplay, mywindow);

  mygc1 = DefaultGC(mydisplay, myscreen); 
	//mygc2 = DefaultGC(mydisplay, myscreen);
}

void *draw() {
	//zmienne pomocnicze
	int x, y, oldX, oldY; //aktualne i stare (poprzednie) pozycje kursora
	int tryb=0; //tryb rysowania (ołówek, pędzel, gumka, etc.)
	int BR = 10; //brush radius - promień pędzla //użyty też w rysowaniu kwadratów o rozm. BRxBR (gumka do ścierania)
	int menu_border = 2;
	//zmienne do 'guzików'
	int btn_height = 30;
	int btn_width = 100;
	int btn_border = 1;
	int btn_gap = 10;
	int kbtn_size = btn_height/2; //rozmiar przycisków do wybierania kolorów
	//button0
	int btn0_x = 10-menu_border;
	int btn0_y = 10-menu_border;
	//button1
	int btn1_x = btn0_x + btn_width + 2*btn_border + btn_gap;
	int btn1_y = btn0_y;
	//button2
	int btn2_x = btn1_x + btn_width + 2*btn_border + btn_gap;
	int btn2_y = btn1_y;
	//button3
	int btn3_x = btn2_x + btn_width + 2*btn_border + btn_gap;
	int btn3_y = btn2_y;
	//kolor podstawowy i drugorzędny
	int wybieranyKolor = -1; //który kolor jest wybierany
	XColor primaryColor, secondaryColor;
	XAllocNamedColor(mydisplay, mycolormap,"black", &primaryColor, &dummy);
  XAllocNamedColor(mydisplay, mycolormap,"white", &secondaryColor, &dummy);
	//k. podst.
	int cbtn0_x = btn0_x; //color button
	int cbtn0_y = btn0_y + btn_height + 2*btn_border + btn_gap;
	//k. drug.
	int cbtn1_x =	cbtn0_x + btn_height + 2*btn_border + btn_gap;
	int cbtn1_y = cbtn0_y;
	//kilka kolorów do wybrania
	//0 - czarny
	int k0_x = cbtn1_x + btn_height + 2*btn_border + btn_gap*2;
	int k0_y = cbtn1_y;
	//1 - bialy
	int k1_x = k0_x + kbtn_size + 2*btn_border + btn_gap;
	int k1_y = k0_y;
	//2 - czerwony
	int k2_x = k1_x + kbtn_size + 2*btn_border + btn_gap;
	int k2_y = k1_y;
	//3 - niebieski
	int k3_x = k2_x + kbtn_size + 2*btn_border + btn_gap;
	int k3_y = k2_y;
	
 	//child window - menu
	XColor menu_color;
	XAllocNamedColor(mydisplay, mycolormap,"Ghost White", &menu_color, &dummy);	
	//
	Window childWindow;
	childWindow = XCreateSimpleWindow(mydisplay, mywindow,
		0, 0, 1236, 100,
		menu_border, BlackPixel(mydisplay, myscreen), menu_color.pixel );
	
	XSelectInput(mydisplay, childWindow, ExposureMask | KeyPressMask);
	XMapWindow(mydisplay, childWindow);
		XColor menuItem_color;
		XAllocNamedColor(mydisplay, mycolormap,"Gray", &menuItem_color, &dummy);
		
		//0 - ołówek
		Window menu0_Win;
		menu0_Win = XCreateSimpleWindow(mydisplay, childWindow,
			btn0_x, btn0_y, btn_width, btn_height,
			btn_border, BlackPixel(mydisplay, myscreen), menuItem_color.pixel );
		
		XSelectInput(mydisplay, menu0_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, menu0_Win);
		//1 - pędzel
		Window menu1_Win;
		menu1_Win = XCreateSimpleWindow(mydisplay, childWindow,
			btn1_x, btn1_y, btn_width, btn_height,
			btn_border, BlackPixel(mydisplay, myscreen), menuItem_color.pixel );
		
		XSelectInput(mydisplay, menu1_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, menu1_Win);
		//2 - gumka
		Window menu2_Win;
		menu2_Win = XCreateSimpleWindow(mydisplay, childWindow,
			btn2_x, btn2_y, btn_width, btn_height,
			btn_border, BlackPixel(mydisplay, myscreen), menuItem_color.pixel );
		
		XSelectInput(mydisplay, menu2_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, menu2_Win);
		//3 - wyczyść główną tablicę rysowania
		Window menu3_Win;
		menu3_Win = XCreateSimpleWindow(mydisplay, childWindow,
			btn3_x, btn3_y, btn_width, btn_height,
			btn_border, BlackPixel(mydisplay, myscreen), menuItem_color.pixel );
		
		XSelectInput(mydisplay, menu3_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, menu3_Win);

		//znacznik koloru podstawowego
		Window menuColor0_Win;
		menuColor0_Win = XCreateSimpleWindow(mydisplay, childWindow,
			cbtn0_x, cbtn0_y, btn_height, btn_height, //bo kwardrat
			btn_border, BlackPixel(mydisplay, myscreen), primaryColor.pixel );
		
		XSelectInput(mydisplay, menuColor0_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, menuColor0_Win);
		//znacznik koloru drugorzędnego
		Window menuColor1_Win;
		menuColor1_Win = XCreateSimpleWindow(mydisplay, childWindow,
			cbtn1_x, cbtn1_y, btn_height, btn_height, //bo kwardrat
			btn_border, BlackPixel(mydisplay, myscreen), secondaryColor.pixel );
		
		XSelectInput(mydisplay, menuColor1_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, menuColor1_Win);

		//kolory do wyboru:
		//0-czarny
		Window k0_Win;
		k0_Win = XCreateSimpleWindow(mydisplay, childWindow,
			k0_x, k0_y, kbtn_size, kbtn_size,
			btn_border, BlackPixel(mydisplay, myscreen), mycolor1.pixel );
		
		XSelectInput(mydisplay, k0_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, k0_Win);
		//1-bialy
		Window k1_Win;
		k1_Win = XCreateSimpleWindow(mydisplay, childWindow,
			k1_x, k1_y, kbtn_size, kbtn_size,
			btn_border, BlackPixel(mydisplay, myscreen), mycolor2.pixel );
		
		XSelectInput(mydisplay, k1_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, k1_Win);
		//2-czerwony
		Window k2_Win;
		k2_Win = XCreateSimpleWindow(mydisplay, childWindow,
			k2_x, k2_y, kbtn_size, kbtn_size,
			btn_border, BlackPixel(mydisplay, myscreen), mycolor3.pixel );
		
		XSelectInput(mydisplay, k2_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, k2_Win);
		//3-niebieski
		Window k3_Win;
		k3_Win = XCreateSimpleWindow(mydisplay, childWindow,
			k3_x, k3_y, kbtn_size, kbtn_size,
			btn_border, BlackPixel(mydisplay, myscreen), mycolor4.pixel );
		
		XSelectInput(mydisplay, k3_Win, ExposureMask | KeyPressMask);
		XMapWindow(mydisplay, k3_Win);
	//child window - menu -> KONIEC

  while (1) {
    XNextEvent(mydisplay, &myevent);

    switch (myevent.type) {
    case Expose:
			//tymczasowe prostokąty
			/* XSetForeground(mydisplay, mygc1, mycolor3.pixel);
			XFillRectangle(mydisplay, mywindow, mygc1, 150, 150, 300, 300);
			XSetForeground(mydisplay, mygc1, mycolor4.pixel);
			XFillRectangle(mydisplay, mywindow, mygc1, 450, 150, 300, 300); */
			
			//napisy na przyciskach
			XSetForeground(mydisplay, mygc1, mycolor1.pixel);
			XFontStruct *font;
			XTextItem ti[1];
			font = XLoadQueryFont(mydisplay, "7x14");
			//olowek
			ti[0].chars = "OLOWEK";
			ti[0].nchars = 6;
			ti[0].delta = 0;
			ti[0].font = font->fid;
			XDrawText(mydisplay, menu0_Win, mygc1,
						(btn_width-XTextWidth(font, ti[0].chars, ti[0].nchars))/2,
						(btn_height-(font->ascent+font->descent))/2 + font->ascent,
						ti, 1);
			//pedzel
			ti[0].chars = "PEDZEL";
			ti[0].nchars = 6;
			XDrawText(mydisplay, menu1_Win, mygc1,
						(btn_width-XTextWidth(font, ti[0].chars, ti[0].nchars))/2,
						(btn_height-(font->ascent+font->descent))/2 + font->ascent,
						ti, 1);
			//gumka
			ti[0].chars = "GUMKA";
			ti[0].nchars = 5;
			XDrawText(mydisplay, menu2_Win, mygc1,
						(btn_width-XTextWidth(font, ti[0].chars, ti[0].nchars))/2,
						(btn_height-(font->ascent+font->descent))/2 + font->ascent,
						ti, 1);
			//clear
			ti[0].chars = "CZYSC";
			ti[0].nchars = 5;
			XDrawText(mydisplay, menu3_Win, mygc1,
						(btn_width-XTextWidth(font, ti[0].chars, ti[0].nchars))/2,
						(btn_height-(font->ascent+font->descent))/2 + font->ascent,
						ti, 1);
			XUnloadFont(mydisplay, font->fid);

      XFlush(mydisplay);
      break;

		case ButtonPress:
			x = myevent.xbutton.x;
    	y = myevent.xbutton.y;
			oldX = x;
			oldY = y;
    	mywindow = myevent.xbutton.window;

			switch (myevent.xbutton.button) {
        case Button1:
						printf("%dx%d (click)\n", x,y);

						//W ZALEZNOSCI JAKI PRZYCISK KLIKNIETO - odpowiednia akcja
						if( x>=btn0_x+menu_border && x<=(btn0_x+btn_width+2*btn_border)+menu_border && y>=btn0_y+menu_border && y<=(btn0_y+btn_height+2*btn_border)+menu_border){	//button0
							tryb=0;
							printf("tryb 0!\n");
						}else if( x>=btn1_x+menu_border && x<=(btn1_x+btn_width+2*btn_border)+menu_border && y>=btn1_y+menu_border && y<=(btn1_y+btn_height+2*btn_border)+menu_border){	//button1
							tryb=1;
							printf("tryb 1!\n");
						}else if( x>=btn2_x+menu_border && x<=(btn2_x+btn_width+2*btn_border)+menu_border && y>=btn2_y+menu_border && y<=(btn2_y+btn_height+2*btn_border)+menu_border){	//button2
							tryb=2;
							printf("tryb 2!\n");
						}else if( x>=btn3_x+menu_border && x<=(btn3_x+btn_width+2*btn_border)+menu_border && y>=btn3_y+menu_border && y<=(btn3_y+btn_height+2*btn_border)+menu_border){	//button3
							tryb=3;
							printf("tryb 3!\n");
						}else if( x>=cbtn0_x+menu_border && x<=(cbtn0_x+btn_height+2*btn_border)+menu_border && y>=cbtn0_y+menu_border && y<=(cbtn0_y+btn_height+2*btn_border)+menu_border){	//kolor podstawowy
							wybieranyKolor=0;
							printf("wybierany kolor: PODST!\n");
						}else if( x>=cbtn1_x+menu_border && x<=(cbtn1_x+btn_height+2*btn_border)+menu_border && y>=cbtn1_y+menu_border && y<=(cbtn1_y+btn_height+2*btn_border)+menu_border){	//kolor drugorzędny
							wybieranyKolor=1;
							printf("wybierany kolor: DRUG!\n");
						}else if( x>=k0_x+menu_border && x<=(k0_x+(kbtn_size)+2*btn_border)+menu_border && y>=k0_y+menu_border && y<=(k0_y+(kbtn_size)+2*btn_border)+menu_border){	//k0- czarny
							printf("kolor: czarny - ");
							if(wybieranyKolor==0){
								primaryColor = mycolor1;
								printf("PRIM\n");
							}else if(wybieranyKolor==1){
								secondaryColor = mycolor1;
								printf("SECO\n");
							}
							wybieranyKolor = -1;
							XSetForeground(mydisplay, mygc1, primaryColor.pixel);
							XFillRectangle(mydisplay, menuColor0_Win, mygc1, 0, 0, btn_height, btn_height);
							XSetForeground(mydisplay, mygc1, secondaryColor.pixel);
							XFillRectangle(mydisplay, menuColor1_Win, mygc1, 0, 0, btn_height, btn_height);
						}else if( x>=k1_x+menu_border && x<=(k1_x+(kbtn_size)+2*btn_border)+menu_border && y>=k1_y+menu_border && y<=(k1_y+(kbtn_size)+2*btn_border)+menu_border){	//k1- biały
							printf("kolor: bialy - ");
							if(wybieranyKolor==0){
								primaryColor = mycolor2;
								printf("PRIM\n");
							}else if(wybieranyKolor==1){
								secondaryColor = mycolor2;
								printf("SECO\n");
							}
							wybieranyKolor = -1;
							XSetForeground(mydisplay, mygc1, primaryColor.pixel);
							XFillRectangle(mydisplay, menuColor0_Win, mygc1, 0, 0, btn_height, btn_height);
							XSetForeground(mydisplay, mygc1, secondaryColor.pixel);
							XFillRectangle(mydisplay, menuColor1_Win, mygc1, 0, 0, btn_height, btn_height);
						}else if( x>=k2_x+menu_border && x<=(k2_x+(kbtn_size)+2*btn_border)+menu_border && y>=k2_y+menu_border && y<=(k2_y+(kbtn_size)+2*btn_border)+menu_border){	//k2- czerwony
							printf("kolor: czerwony - ");
							if(wybieranyKolor==0){
								primaryColor = mycolor3;
								printf("PRIM\n");
							}else if(wybieranyKolor==1){
								secondaryColor = mycolor3;
								printf("SECO\n");
							}
							wybieranyKolor = -1;
							XSetForeground(mydisplay, mygc1, primaryColor.pixel);
							XFillRectangle(mydisplay, menuColor0_Win, mygc1, 0, 0, btn_height, btn_height);
							XSetForeground(mydisplay, mygc1, secondaryColor.pixel);
							XFillRectangle(mydisplay, menuColor1_Win, mygc1, 0, 0, btn_height, btn_height);
						}else if( x>=k3_x+menu_border && x<=(k3_x+(kbtn_size)+2*btn_border)+menu_border && y>=k3_y+menu_border && y<=(k3_y+(kbtn_size)+2*btn_border)+menu_border){	//k2- niebieski
							printf("kolor: niebieski - ");
							if(wybieranyKolor==0){
								primaryColor = mycolor4;
								printf("PRIM\n");
							}else if(wybieranyKolor==1){
								secondaryColor = mycolor4;
								printf("SECO\n");
							}
							wybieranyKolor = -1;
							XSetForeground(mydisplay, mygc1, primaryColor.pixel);
							XFillRectangle(mydisplay, menuColor0_Win, mygc1, 0, 0, btn_height, btn_height);
							XSetForeground(mydisplay, mygc1, secondaryColor.pixel);
							XFillRectangle(mydisplay, menuColor1_Win, mygc1, 0, 0, btn_height, btn_height);
						}
						XSetForeground(mydisplay, mygc1, primaryColor.pixel);
						switch(tryb){
							case 0:
								XDrawPoint(mydisplay, mywindow, mygc1, x, y); 
								break;
							case 1:
								/*for(int i=0; i<BR; i++){
									XDrawArc(mydisplay, mywindow, mygc1, x-(i/2), y-(i/2), i, i, 0, 360*64);
								}*/
								XFillArc(mydisplay, mywindow, mygc1, x-(BR/2), y-(BR/2), BR, BR, 0, 360*64);
								break;
							case 2:
								XSetForeground(mydisplay, mygc1, secondaryColor.pixel);
								XFillRectangle(mydisplay, mywindow, mygc1, x-(BR/2), y-(BR/2), BR, BR);
								break;
						}
            break;
        default: //inne przyciski
            break;
    	}
			break;
		case ButtonRelease:
			//d1.image = XGetImage(mydisplay, mywindow, 0,0, 1240, 874, AllPlanes, XYPixmap);
			if(tryb==3){
				printf("clear!\n");
				XClearArea(mydisplay, mywindow, 0, 0, 1240, 874, 0);			
				//XFillRectangle(mydisplay, childWindow, mygc1, 0, 0, 20, 20);	
			}
			//rysowanie zaznaczenia który kolor jest wybierany
			//i jednoczesne usunięcie (zamazanie) znacznika na innym kolorze
			if(wybieranyKolor==0){
				XSetForeground(mydisplay, mygc1, mycolor3.pixel);
				XFillArc(mydisplay, menuColor0_Win, mygc1, (0+(btn_height/2))-5, (0+(btn_height/2))-5, 10, 10, 0, 360*64);	
				XSetForeground(mydisplay, mygc1, mycolor4.pixel);
				XFillArc(mydisplay, menuColor0_Win, mygc1, (0+(btn_height/2))-3, (0+(btn_height/2))-3, 6, 6, 0, 360*64);
				//zamazanie
				XSetForeground(mydisplay, mygc1, secondaryColor.pixel);
				XFillRectangle(mydisplay, menuColor1_Win, mygc1, 0, 0, btn_height, btn_height);
			}else if(wybieranyKolor==1){
				XSetForeground(mydisplay, mygc1, mycolor3.pixel);
				XFillArc(mydisplay, menuColor1_Win, mygc1, (0+(btn_height/2))-5, (0+(btn_height/2))-5, 10, 10, 0, 360*64);	
				XSetForeground(mydisplay, mygc1, mycolor4.pixel);
				XFillArc(mydisplay, menuColor1_Win, mygc1, (0+(btn_height/2))-3, (0+(btn_height/2))-3, 6, 6, 0, 360*64);	
				//zamazanie
				XSetForeground(mydisplay, mygc1, primaryColor.pixel);
				XFillRectangle(mydisplay, menuColor0_Win, mygc1, 0, 0, btn_height, btn_height);
			}
			d1.image = XGetImage(mydisplay, mywindow, 0,0, 1240, 874, AllPlanes, XYPixmap);
			sendto(socket_descriptor, (char *) &(d1), sizeof(d1), 0, (struct sockaddr *) &recipient,size);
			break;
		case MotionNotify:
			x = myevent.xmotion.x;
			y = myevent.xmotion.y;
			mywindow = myevent.xbutton.window;
		
			//LPM
			if(myevent.xmotion.state & Button1Mask){
				XSetForeground(mydisplay, mygc1, primaryColor.pixel);
				switch(tryb){
					case 0:			
						XDrawLine(mydisplay, mywindow, mygc1, oldX, oldY, x, y);
						//XDrawPoint(mydisplay, mywindow, mygc1, x, y);	
						break;
					case 1:
						//rysowanie linii od wszystkich pozycji w promieniu (x,y) do wszystkich pozycji w promieniu (oldX, oldY)
						for(int i=x-(BR/2); i<=x+(BR/2); i++){
							for(int j=y-(BR/2); j<=y+(BR/2); j++){
								//punkt który leży w promieniu od punktu(x,y) - zawiera się w kole o promieniu
								if( ( ((i-x)*(i-x) + (j-y)*(j-y)) <= (BR/2)*(BR/2) ) ){
									for(int i2=oldX-(BR/2); i2<=oldX+(BR/2); i2++){
										for(int j2=oldY-(BR/2); j2<=oldY+(BR/2); j2++){
											//
											if( ( ((i2-oldX)*(i2-oldX) + (j2-oldY)*(j2-oldY)) <= (BR/2)*(BR/2) ) ){
												XDrawLine(mydisplay, mywindow, mygc1, i2, j2, i, j);
											}
										}
									}
								}
							}
						}
						break;
					case 2:
						XSetForeground(mydisplay, mygc1, secondaryColor.pixel);
						for(int i=x-(BR/2); i<=x+(BR/2); i++){
							for(int j=y-(BR/2); j<=y+(BR/2); j++){
								//punkt który leży w promieniu od punktu(x,y) - zawiera się w kole o promieniu
								if( i==(x-(BR/2)) || j==(y-(BR/2)) || i==(x+(BR/2)) || j==(y+(BR/2))){ //obramowanie kwadratu
									for(int i2=oldX-(BR/2); i2<=oldX+(BR/2); i2++){
										for(int j2=oldY-(BR/2); j2<=oldY+(BR/2); j2++){
										//punkt który leży w promieniu od punktu(x,y) - zawiera się w kole o promieniu
											if( i2==(oldX-(BR/2)) || j2==(oldY-(BR/2)) || i2==(oldX+(BR/2)) || j2==(oldY+(BR/2))){ //obramowanie kwadratu
												XDrawLine(mydisplay, mywindow, mygc1, i2, j2, i, j);
											}
										}
									}
								}
							}
						}
						break;
				}
			}
			oldX = x;
			oldY = y;
			break;
		}
  }
}

int main(int argc, char **argv) {
	d1.userId=-1;

	//d1.image = 100;
	otworzOkno();
	d1.image = XGetImage(mydisplay, mywindow, 0,0, 1240, 874, AllPlanes, XYPixmap);

	pthread_t tid;
  if (argc != 2) {
       printf("Niepoprawne wywolanie programu.\n");
       printf("./klient ADRES_IP_SERWERA\n");
       return -1;
   }
   size = sizeof(struct sockaddr_in);
   recipient.sin_family = AF_INET;
   recipient.sin_port = htons((ushort) 5000);
   if(inet_pton(AF_INET, argv[1], &recipient.sin_addr)!=1){
       fprintf(stderr, "Blad polaczenia!\n");
       exit(0);
  }

	socket_descriptor = socket(PF_INET, SOCK_DGRAM, 0);
  bind(socket_descriptor, (struct sockaddr *) &recipient, size);

	sendto(socket_descriptor, (char *) &(d1), sizeof(d1), 0, (struct sockaddr *) &recipient, size);
	recvfrom(socket_descriptor, &(d1), sizeof(d1), 0, (struct sockaddr *) &recipient, &size);
	printf("otrzymalem id od serwera: %d\n", d1.userId);

	pthread_create(&tid, NULL, draw, NULL);
	while (1) {
   	recvfrom(socket_descriptor, &(d1), sizeof(d1), 0, (struct sockaddr *) &recipient, &size);
    //drawGame();
		//printf("%d\n", d1.image);
		XPutImage(mydisplay, mywindow, mygc1, d1.image, 0,0, 0,0, 1240,874);
 }

	return 0;
}
