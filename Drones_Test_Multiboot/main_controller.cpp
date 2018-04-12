#include <drone_test/keyboard_controller.h>
#include <iostream>
#include <ncurses.h>
#include <unistd.h> 



//position couleur
int couleur_x;
int couleur_y;
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
int y_max = 250;
//tracking actif a 1
int track=0;
//vitesse de deplacement
float vitesse_x = 0.0;
float vitesse_y = 0.0;
float taille_h;

//recuperation de la position couleur
void  posCallback(const geometry_msgs::Point myPos)
{
	couleur_x=myPos.x;
	couleur_y = myPos.y;
	taille_h = myPos.z;
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
  ros::Subscriber sub = n.subscribe("/cible", 1000, posCallback);


  DroneController bebop;
  float avancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00; 
  initscr();

  cbreak();
  noecho();
  nodelay(stdscr, TRUE);

  while(ros::ok())
  {
   if (keypressed()) 			// a key is pressed
   { 
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
			  avancement =  0.1;
			  break;

		  case 107: // k: reculer
			 avancement =  -0.1;
			  break;

		  case 97: // a: rotation gauche
			  rotation =  0.1;
			   break;

		  case 100: // d:rotation doite
			  rotation =  -0.1;
			   break;

		  case 106: // j:translation gauche
			  translation =  0.1;
			  break;

		  case 108: // l:translation droite
			  translation =  -0.1;
			  break;

		  case 119: // w:monter en altitude
			  hauteur =  0.1;
			   break;

		  case 115: // s:descendre en altitude
			 hauteur =  -0.5;
			  break;
		
		  case 118: // v: track or stop tracking
			  if(track!=1){
			  track=1;
			  }
			  else if(track!=0){
			  track=0;
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
			printf("couleur X %d\n\r",couleur_x);
			printf("couleur y %d\n\r" ,couleur_y);
			printf("taille  %f\n\r",taille_h);

			//calcul vitesse de deplacement en hauteur
			if(couleur_y<y_min){
				vitesse_y = 0.1;
				printf("monter \n\r");
			}else if(couleur_y>y_max){
				vitesse_y = -0.1;
				printf("descendre \n\r");
			}else{
				vitesse_y = 0;
				printf("altitude OK \n\r");
			}

			printf("changement d'altitude %f\n\r",vitesse_y);
			hauteur = vitesse_y;

			//calcul vitesse de deplacement en rotation
			if(couleur_x<x_min){
				vitesse_x = 0.1*((x_min-couleur_x)/50);
				printf("tourner gauche \n\r");
			}else if(x_max<couleur_x){
				vitesse_x = -0.1*((couleur_x-x_max)/50);
				printf("tourner droite \n\r");
			}else{
				vitesse_x = 0;
				printf("ne pas tourner \n\r");
			}

			printf("vitesse de rotation %f\n\r",vitesse_x);
			rotation = vitesse_x;

			//calcul vitesse de deplacement avant/arrière
					if((taille_h>2500)&&(taille_h<10000)){
					printf("rester\n\r");
					avancement=0;
			}
					if(taille_h<2500){
				printf("avancer\n\r");
				avancement=0.1;
			}
					if(taille_h>10000){
				printf("reculer\n\r");
				avancement=-0.1;
			}
					if(taille_h<100){
			// a key is pressed				avancement=0;
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
