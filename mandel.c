#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>


#define WIDTH 1000.0
#define HEIGHT 600.0
int MAX_ITER= 50;

double out_max_x;
double out_min_x;
double out_max_y= 2;
double out_min_y=-2;
int zoom_forever = 0;


int draw(SDL_Renderer **,int);
double map(double,double ,double, double, double);
void change_viewport_wrt_mouse(int,int,float,float);
int handle_key_presses(int,float,float,int,int);
void white_paint_and_draw(SDL_Renderer**, int*);

int main(int argc, char *argv[])
{
  out_min_x = -2 * WIDTH/HEIGHT;
  out_max_x = 2 * WIDTH/HEIGHT;
  if (SDL_Init(SDL_INIT_VIDEO))
    {
	printf ("SDL_Init Error: %s", SDL_GetError());
	return 1;
    }
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    window = SDL_CreateWindow("Mandelbrot Set", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
	printf ("SDL_CreateWindow Error: %s", SDL_GetError());
	SDL_Quit();
	return 2;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
	SDL_DestroyWindow(window);
	printf ("SDL_CreateRenderer Error: %s", SDL_GetError());
	SDL_Quit();
	return 3;
    }

    SDL_Event event;
    int quit = 0;

    //Factor is a random number that will spice things up for the image.
    int factor = 10;

    //Default value of to_render is true and is set true again when the user does some action scrolls in or moves the frame
    int to_draw;

    //Clear using white color before going inside the loop and set to_draw to 1
    white_paint_and_draw(&renderer,&to_draw);

    //Relative position of mouse_x and mouse_y
    int mouse_x, mouse_y;
    // offsets to zoom in or out or move image sidewise
    float offset_x,offset_y;
    while (!quit){
      offset_x = (out_max_x - out_min_x);
      offset_y = (out_max_y - out_min_y);
      while (SDL_PollEvent(&event))
	  {
	  SDL_GetMouseState(&mouse_x,&mouse_y);
	   switch (event.type) {
	   case SDL_QUIT: 
	     quit = 1;
	     break;
	   case SDL_MOUSEWHEEL:
	     if(event.wheel.y > 0)
	       // scroll down
	       {
		 printf("\r%-100s","Zooming in on mouse pointer. Wait for image to render!");
		 fflush(stdout);
		 offset_x /= 4;
		 offset_y /= 4;
		 MAX_ITER += 20;
	       }else if (event.wheel.y < 0)
	       // scroll up
	       {
		 printf("\r%-100s","Zooming out. Wait for image to render!");
		 fflush(stdout);
		 offset_x *=2; 
		 offset_y *=2;
		 if (MAX_ITER >= 50) {
		   MAX_ITER -= 10;		   
		 }

	       }
	       change_viewport_wrt_mouse(mouse_x,mouse_y,offset_x,offset_y);
	       white_paint_and_draw(&renderer,&to_draw);
	       break;
	   case SDL_KEYDOWN:
	     //if only designated keys are pressed than draw
	       if (handle_key_presses(event.key.keysym.sym,offset_x,offset_y,mouse_x,mouse_y)) {		 
		 white_paint_and_draw(&renderer,&to_draw);
	       }
	       break;
	   }

	  }

      if (zoom_forever) {
      //Decreasing and increasing values by certain Percenatage of the offsets for unifom scaling
      //And preveting the values to get reversed in sign.
	out_min_y += offset_y*0.20;
	out_max_y -= offset_y*0.20;
	out_min_x += offset_x*0.20;
	out_max_x -= offset_x*0.20;
	white_paint_and_draw(&renderer,&to_draw);
      }


	//Draw pixels on the renderer
	if (to_draw) {
	  to_draw = draw(&renderer,factor);	    
	  SDL_RenderPresent(renderer);
	  printf("\r%-100s","Image Rendered! You may now zoom or pan.");
	  if (zoom_forever) {
	    printf("\r%-100s","Zooming in... Press G to stop.");
	  }
	  fflush(stdout);
	}
    }

    //free resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);      
    SDL_Quit();
    return 0;
}

int draw(SDL_Renderer **renderer,int factor){
   for (int x = 0; x < WIDTH; x++) {
      for (int y =0;  y < HEIGHT; y++) {
      // Mapping the screen with the limits.
      // Same scaling has been done such that the image is centered on the screen.
	double c_real = out_min_x + (out_max_y-out_min_y)/(HEIGHT)*x; 
	double c_img = map(y,0,HEIGHT, out_min_y,out_max_y); 

	double z_real = 0;
	double z_img = 0;
	int iter_count = 0;
	while (pow(z_real,2)+pow(z_img,2) <= 4 && iter_count < MAX_ITER) {
	  double temp_real = pow(z_real,2)-pow(z_img,2)+c_real;
	  double temp_img = 2*z_real*z_img + c_img;
	  z_real = temp_real;
	  z_img = temp_img;
	  iter_count++;
	}

	//If any number exits before reaching MAX_ITER then, it is not in the set. So colour it with different shade.
	if (iter_count == MAX_ITER) {
	  //printf("SELECT %.2f %.2f %d %d\n",c_real,c_img,x,y);
	  //Draw with black
	  SDL_SetRenderDrawColor(*renderer, 0,0, 0, SDL_ALPHA_OPAQUE);
	  SDL_RenderDrawPoint(*renderer,x,y);
	}else{
	   //Draw with custom shade
	  SDL_SetRenderDrawColor(*renderer, iter_count*factor*5,iter_count*factor, iter_count*factor, SDL_ALPHA_OPAQUE);
	  SDL_RenderDrawPoint(*renderer,x,y);
	}
      }
   }
   return 0;
}

void change_viewport_wrt_mouse(int mouse_x,int mouse_y,float offset_x, float offset_y){
  double mouse_x_mapped = out_min_x + (out_max_y-out_min_y)/(HEIGHT)*mouse_x;  
  double mouse_y_mapped = map(mouse_y,0,HEIGHT, out_min_y,out_max_y);
  out_min_x = mouse_x_mapped - offset_x;
  out_max_x = mouse_x_mapped + offset_x;
  out_min_y = mouse_y_mapped - offset_y;
  out_max_y = mouse_y_mapped + offset_y;

}

int handle_key_presses(int keycode,float offset_x, float offset_y,int mouse_x,int mouse_y){
   switch (keycode)
     {
     case SDLK_w:
       //Move up
       //Since y axis is inverted subtracting will take us to upper part of screen
       printf("\r%-100s","Moving up. Wait for image to render!");
       out_min_y -= offset_y/4;
       out_max_y -= offset_y/4;
       break;
     case SDLK_s:
       //Move down
       printf("\r%-100s","Moving down. Wait for image to render!");
       out_min_y += offset_y/4;
       out_max_y += offset_y/4;
       break;
     case SDLK_a:
       //Move left
       printf("\r%-100s","Moving Left. Wait for image to render!");
       out_min_x -= offset_x/4;
       out_max_x -= offset_x/4;
       break;
     case SDLK_d:
       //Move right
       printf("\r%-100s","Moving Right. Wait for image to render!");
       out_min_x += offset_x/4;
       out_max_x += offset_x/4;
       break;
     case SDLK_SPACE:
       //Zoom in
       printf("\r%-100s","Zooming in on mouse pointer. Wait for image to render!");
       change_viewport_wrt_mouse(mouse_x,mouse_y,offset_x/4,offset_y/4);
       break;
     case SDLK_f:
       //Zoom forever
       printf("\r%-100s","Zooming forever on first mouse pointer location. Wait for image to render!");
       zoom_forever = 1;
       //Center the point under mouse pointer.
       change_viewport_wrt_mouse(mouse_x,mouse_y,offset_x/2,offset_y/2);
       break;
     case SDLK_g:
       //Stop Zoom forever
       printf("\r%-100s","Zooming forever stopped!");
       zoom_forever = 0;
       break;
     case SDLK_c:
       //Center the point under the mouse pointer.
       printf("\r%-100s","Centering the point under mouse pointer. Wait for image to render!");
       change_viewport_wrt_mouse(mouse_x,mouse_y,offset_x/2,offset_y/2);
       break;
     default:
       return 0;
     }
   return 1;
}

void white_paint_and_draw(SDL_Renderer **renderer, int *to_draw){
   SDL_SetRenderDrawColor(*renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
   SDL_RenderClear(*renderer);
   *to_draw = 1;
}

double map(double input_value, double input_min, double input_max, double output_min, double output_max){
  return output_min + (output_max-output_min)/(input_max-input_min)*(input_value-input_min);
}
