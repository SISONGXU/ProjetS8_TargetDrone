#include <drone_test/keyboard_controller.h>
#include <iostream>
#include <ncurses.h>
#include <unistd.h> 



//position couleur
int couleur_x;
int couleur_y;
//position face
int face_x;
int face_y;
//distance entre couleur et drone
int distance_max=100;
int distance_x=400;
int distance_y=200;
//taille image
int taille_image=856;
//position centrale de l'ecran
//int x_min = taille_image*(5/11);
//int x_max = taille_image*(6/11);
int x_min = 350;
int x_max = 450;
//int pos y 
int y_min = 200;
int y_max = 250;
//tracking actif a 1
int track=0;
//vitesse de deplacement
float vitesse_x = 0.0;
float vitesse_y = 0.0;
float taille_couleur=0;
float taille_couleur_origin=0;
float taille_face=0;
float taille_face_origin=0;
float yaw;
int check=0;


//recuperation de la position couleur
void  posCallback(const geometry_msgs::Point myPos)
{
	couleur_x=myPos.x;
	couleur_y = myPos.y;
        if(check==0){
        taille_couleur_origin = myPos.z;
        check=1;}
        if(check==1){
        taille_couleur = myPos.z;
      }   
}

void  posCallback1(const geometry_msgs::Point myface)
{
	face_x=myface.x;
	face_y = myface.y;
        
        if(check==0){
        taille_face_origin=myface.z;
        check=1;}
        if(check==1){
        taille_face = myface.z;
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
  ros::Subscriber sub = n.subscribe("/cible", 1000, posCallback);
  ros::Subscriber sub1 = n.subscribe("/cible_face", 1000, posCallback1);

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

		  case 115: // s:descendre en altitude
			 hauteur =  -0.5;
			  break;
                   
                  case 114:// r:reset check
		          check=0;
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
			printf("taille couleur %f\n\r",taille_couleur);
                        printf("taille couleur origin %f\n\r",taille_couleur_origin);

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
					if((taille_couleur>taille_couleur_origin*0.7)&&(taille_couleur<taille_couleur_origin*1.3)){
					printf("rester\n\r");
					avancement=0;
			}
					if((taille_couleur<taille_couleur_origin*0.7)&&(taille_couleur>100)){
				printf("avancer\n\r");
				avancement=0.1;
			}
					if(taille_couleur>taille_couleur_origin*1.3){
				printf("reculer\n\r");
				avancement=-0.1;
			}
					if(taille_couleur<100){
			// a key is pressed				avancement=0;
			printf("Cilbe perdu\n\r");
			}
			printf("vitesse avancement %f\n\r",avancement);	

		}
		printf("\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r");

/*

                if(track!=0)
		{
			printf("face X %d\n\r",face_x);
			printf("face y %d\n\r" ,face_y);
			printf("taille face %f\n\r",taille_face);
                        printf("taille face origin %f\n\r",taille_face_origin);
			//calcul vitesse de deplacement en hauteur
			if(face_y<y_min){
				vitesse_y = 0.1;
				printf("monter \n\r");
			}else if(face_y>y_max){
				vitesse_y = -0.1;
				printf("descendre \n\r");
			}else{
				vitesse_y = 0;
				printf("altitude OK \n\r");
			}

			printf("changement d'altitude %f\n\r",vitesse_y);
			hauteur = vitesse_y;

			//calcul vitesse de deplacement en rotation
			if(face_x<x_min){
				vitesse_x = 0.1;
				printf("tourner gauche \n\r");
			}else if(x_max<face_x){
				vitesse_x = -0.1;
				printf("tourner droite \n\r");
			}else{
				vitesse_x = 0;
				printf("ne pas tourner \n\r");
			}

			printf("vitesse de rotation %f\n\r",vitesse_x);
			rotation = vitesse_x;

			//calcul vitesse de deplacement avant/arrière
					//if((taille_face>2500)&&(taille_face<10000)){

			if(taille_face==0){
			// a key is pressed				avancement=0;
			printf("Cilbe perdu\n\r");
                         avancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00; 
			}
                        else{

                                if((taille_face>taille_face_origin*0.8)&&(taille_face<taille_face_origin*1.2)){
                                printf("rester\n\r");
	               		avancement=0;
			}
				if(taille_face<taille_face_origin*0.8){
				printf("avancer\n\r");
				avancement=0.1;
			}
				if(taille_face>taille_face_origin*1.2){
				printf("reculer\n\r");
				avancement=-0.1;
			}
}
			printf("vitesse avancement %f\n\r",avancement);	
		}
		printf("\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r");*/
	 }
	 bebop.setCommand(avancement, translation, hauteur, rotation);
	 avancement = 0.00, translation = 0.00, hauteur = 0.00, rotation = 0.00; 
	 ros::spinOnce(); 
	 loop_rate.sleep();  
  }
  endwin();
}
