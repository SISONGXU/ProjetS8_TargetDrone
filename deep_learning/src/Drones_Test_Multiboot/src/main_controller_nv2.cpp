#include <drone_test/keyboard_controller.h>
#include <iostream>
#include <ncurses.h>
#include <unistd.h> 
#include <ios>
#include <string>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/Quaternion.h>
#include <fstream>
#include <vector>



//position couleur
int centre_x;
int centre_y;
//position avec rectangle
int rectangle_x1;
int rectangle_y1;
int rectangle_x2;
int rectangle_y2;
//distance entre couleur et drone
int distance_max=100;
int distance_x=400;
int distance_y=200;
//taille image
int taille_image=856;
//position centrale de l'ecran
//int x_min = taille_image*(5/11);
//int x_max = taille_image*(6/11);
int x_min = 375;
int x_max = 425;
//int pos y 
int y_min = 150;
int y_max = 280;
//tracking actif a 1
int track=0;
// taille initiale
int taille_init;
//vitesse de deplacement
float vitesse_x = 0.0;
float vitesse_y = 0.0;
float taille_h;

//timer zoom
int zoom_timer;
// valeur par défaut au décollage
float avancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00;

//recuperation de la position couleur
void  posCallback(const geometry_msgs::Point myPos)
{
	centre_x=myPos.x;
	centre_y = myPos.y;
	taille_h = myPos.z;
}

//recuperation de la position rectangle
void posCallbackRectangle(const geometry_msgs::Quaternion pos_h)
{
	rectangle_x1 = pos_h.x;
	rectangle_y1 = pos_h.y;
	rectangle_x2 = pos_h.z;
	rectangle_y2 = pos_h.w;
	
	
	
}

int posCallbackDeepLearning(void)
{
	int x1, y1, x2, y2;
	std::ifstream infile;
	
	if (infile.fail())
	{
		std::cerr << "File can't be opened!" << std::endl;
	}else{
		infile.open("src/deep_learning/src/rectangle.txt");
		infile >> x1 >> y1 >> x2 >> y2;
		rectangle_x1 = x1; 
		rectangle_y1 = y1;
		rectangle_x2 = x2;
		rectangle_y2 = y2;
	}
}

int keypressed(void)
{
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char **argv)
{

  ros::init(argc, argv, "autocontroller");
  ros::NodeHandle n;
  ros::Rate loop_rate(30);
  ros::Subscriber sub = n.subscribe("/cible_humaine", 1000, posCallbackRectangle); //changement


  DroneController bebop;
 
  initscr();

  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  zoom_timer=0;
  while(ros::ok())
  {
	  if (zoom_timer!=0){
		  if(zoom_timer!=30){
			  zoom_timer++;
		  }
		  else{
			  zoom_timer=0;}
	  }
	  
	// Uncomment to use the Deep Learning box positions
	posCallbackDeepLearning();
	
	centre_x = (rectangle_x1 + rectangle_x2)/2;
	centre_y = (rectangle_y1 + rectangle_y2)/2;
	taille_h = (rectangle_x2-rectangle_x1)*(rectangle_y2-rectangle_y1);
	  
   if (keypressed()) 			// a key is pressed
   {
	   //reset all movement variables when a key is pressed to avoid conflict with the tracking mode
	   
	   switch(getch()) // ASCII values
		{
		  case 116: // t: decoller
			  bebop.sendTakeoff();
			  track=0;
			  break;

		  case 110: // n: atterrir
		 	 bebop.sendLand();	
			  track=0;
			  break;

		  case 105: // i: avancer
			  avancement =  0.3;
			  break;

		  case 107: // k: reculer
			 avancement =  -0.3;
			  break;

		  case 97: // a: rotation gauche
			  rotation =  0.3;
			   break;

		  case 100: // d:rotation doite
			  rotation =  -0.3;
			   break;

		  case 106: // j:translation gauche
			  translation =  0.3;
			  break;

		  case 108: // l:translation droite
			  translation =  -0.3;
			  break;

		  case 119: // w:monter en altitude
			  hauteur =  0.3;
			   break;

		  case 115: // s:descendre en altitudeavancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00;
			 hauteur =  -0.5;
			  break;
		
		  case 118: // v: track or stop tracking and set size
			  if(track!=1){
			  track=1;
			  taille_init=taille_h;
			  }
			  else if(track!=0){
			  track=0;
			  }
			   break;
		   case 98: // b: zooavancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00;m in
			   if (zoom_timer==0){
				   taille_init=taille_init*1.1;
				   zoom_timer++;
			   }
			   break;avancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00; 
		   case 99: // c:zoom out
			   if(zoom_timer==0){
				   taille_init=taille_init*0.9;
				   zoom_timer++;
			   }
			  break;
	      default:
              break;
		}
	 
         flushinp(); // on vide le buffer de getch
     }
     else  // a key is not pressed
     {
		if(track!=0)
		{
			printf("cible X %d\n\r",centre_x);
			printf("cible y %d\n\r" ,centre_y);
			printf("taille  %f\n\r",taille_h);

			//calcul vitesse de deplacement en hauteur
			if(centre_y<y_min){
				vitesse_y = 0.13;   
				printf("monter \n\r");
			}else if(centre_y>y_max){
				vitesse_y = -0.13;
				printf("descendre \n\r");
			}else{
				vitesse_y = 0;
				printf("altitude OK \n\r");
			}

			printf("changement d'altitude %f\n\r",vitesse_y);
			hauteur = vitesse_y;

			//calcul vitesse de deplacement en rotation
			if(centre_x<x_min){
				vitesse_x = 0.13;
				//vitesse_x = 0.13*((x_min-centre_x)/50);
				printf("tourner gauche \n\r");
			}else if(x_max<centre_x){
				vitesse_x = -0.13*((centre_x-x_max)/50);
				printf("tourner droite \n\r");
			}else{
				vitesse_x = 0;
				printf("ne pas tourner \n\r");
			}

			printf("vitesse de rotation %f\n\r",vitesse_x);
			rotation = vitesse_x;

			//calcul vitesse de deplacement avant/arrière
					if((taille_h>0.8*taille_init)&&(taille_h<1.2*taille_init)){
					printf("rester\n\r");
					avancement=0;
			}
					if(taille_h<taille_init*0.8){
				printf("avancer\n\r");
				avancement=0.13;
			}
					if(taille_h>taille_init*1.2){
				printf("reculer\n\r");
				avancement=-0.13;
			}
					if(taille_h<100){			
					avancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00;
					printf("Cilbe perdu\n\r");
			}
			printf("vitesse avancement %f\n\r",avancement);	

		}
		printf("\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r");
	 }
	 bebop.setCommand(avancement, translation, hauteur, rotation);
	 avancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00; 
	 ros::spinOnce(); 
	 loop_rate.sleep();  
  }
  endwin();
}
