#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#define WIDTH 1000
#define HEIGHT 600
int MAX_ITER= 50;

long double out_max_x;
long double out_min_x;
long double out_max_y= 2;
long double out_min_y=-2;
int zoom_forever = 0;


typedef struct app{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  uint32_t *pixels;
}App;

int draw(App*);
void change_viewport_wrt_mouse(int,int,long double,long double);
int handle_key_presses(int,long double,long double,int,int);
int initialize_everything(App*);
void destroy_app(App*);
void main_loop(App*);

int main(int argc, char *argv[])
{

  App my_app = {NULL,NULL,NULL,NULL};
  if(initialize_everything(&my_app)) return 1;
  main_loop(&my_app);
  printf("\n");
  destroy_app(&my_app);
  return 0;
}

int initialize_everything(App *my_app){
  out_min_x = -2 * WIDTH/HEIGHT;
  out_max_x = 2 * WIDTH/HEIGHT;
  my_app->pixels = (uint32_t*)malloc(WIDTH*HEIGHT*sizeof(uint32_t));
  if (my_app->pixels == NULL){
  {
      printf ("Memory Error: Value of error %d", errno);
      return 1;
    }

  }
  if (SDL_Init(SDL_INIT_EVERYTHING))
    {
      printf ("SDL_Init Error: %s", SDL_GetError());
      return 1;
    }

  my_app->window = SDL_CreateWindow("Mandelbrot Set", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
  if (my_app->window == NULL)
    {
      printf ("SDL_CreateWindow Error: %s", SDL_GetError());
      SDL_Quit();
      return 2;
    }

  my_app->renderer = SDL_CreateRenderer(my_app->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (my_app->renderer == NULL)
    {
      SDL_DestroyWindow(my_app->window);
      printf ("SDL_CreateRenderer Error: %s", SDL_GetError());
      SDL_Quit();
      return 3;
    }
  my_app->texture = SDL_CreateTexture(my_app->renderer,SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);

  if (my_app->texture == NULL)
    {
      SDL_DestroyRenderer(my_app->renderer);
      SDL_DestroyWindow(my_app->window);
      printf ("SDL_Create texture Error: %s", SDL_GetError());
      SDL_Quit();
      return 4;
    }
  return 0;
}

void main_loop(App *my_app){
    SDL_Event event;
      int quit = 0;
      //Default value of to_render is true and is set true again when the user does some action scrolls in or moves the frame
      int to_draw = 1;
      //No need to white paint because entire pixel map is upgraded each time.
      //Relative position of mouse_x and mouse_y
      int mouse_x, mouse_y;
      //Offsets
      long double offset_x,offset_y;

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
		   printf("\r%-110s","Zooming in on mouse pointer. Wait for image to render!");
		   fflush(stdout);
		   offset_x /= 4;
		   offset_y /= 4;
		   MAX_ITER += 20;
		 }else if (event.wheel.y < 0)
		 // scroll up
		 {
		   printf("\r%-110s","Zooming out. Wait for image to render!");
		   fflush(stdout);
		   offset_x *=1.5; 
		   offset_y *=1.5;
		   if (MAX_ITER >= 50) {
		     MAX_ITER -= 20;		   
		   }

		 }
		 change_viewport_wrt_mouse(mouse_x,mouse_y,offset_x,offset_y);
		 to_draw = 1;
		 break;
	     case SDL_KEYDOWN:
	       //if only designated keys are pressed than draw
		 if (handle_key_presses(event.key.keysym.sym,offset_x,offset_y,mouse_x,mouse_y)) {		 
		   to_draw = 1;
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
	  MAX_ITER += 20;	  
	  to_draw = 1;
	}
	//Draw pixels on the renderer
	if (to_draw) {
	  long double time_spent_on_drawing = 0.0;
	  long double time_spent_on_rendering = 0.0;
	  clock_t begin = clock();
	  to_draw = draw(my_app);
	  clock_t end = clock();
	  SDL_UpdateTexture(my_app->texture, NULL, my_app->pixels, WIDTH * sizeof(Uint32));	    
	  time_spent_on_drawing = ((end - begin) /(long double)CLOCKS_PER_SEC);
	  SDL_RenderClear(my_app->renderer);
	  SDL_RenderCopy(my_app->renderer, my_app->texture, NULL, NULL);
	  SDL_RenderPresent(my_app->renderer);
	  clock_t end_two = clock();
	  time_spent_on_rendering = ((end_two - end) /(long double)CLOCKS_PER_SEC);

	  printf("\r%s. %.6LGs to compute pixels and  %.6LGs to render. Total = %.6LGs ","Image Rendered! You may now zoom or pan.",time_spent_on_drawing,time_spent_on_rendering,time_spent_on_drawing+time_spent_on_rendering);
	  if (zoom_forever) {
	    printf("\r%-110s","Zooming in... Press G to stop.");
	  }
	  fflush(stdout);
	}
    }
}

int draw(App* my_app){
   for (int x = 0; x < WIDTH; x++) {
      for (int y =0;  y < HEIGHT; y++) {
      // Mmy_apping the screen with the limits.
      // Same scaling has been done such that the image is centered on the screen.
	long double c_real = out_min_x + (out_max_y-out_min_y)/(HEIGHT)*x; 
	long double c_img =  out_min_y + (out_max_y-out_min_y)/(HEIGHT)*y; 
	long double z_real_squared = 0;
	long double z_img_squared = 0;
	long double z_real = 0;
	long double z_img = 0;
	int iter_count = 0;
	//Trying to reduce multiplication count to 3 per iteration
	while (z_real_squared+z_img_squared <= 4 && iter_count < MAX_ITER) {
	  z_img = 2*z_real*z_img + c_img;
	  z_real = z_real_squared-z_img_squared+c_real;
	  z_real_squared=z_real*z_real;
	  z_img_squared = z_img*z_img;
	  iter_count++;
	}

	//If any number exits before reaching MAX_ITER then, it is not in the set. So colour it with different shade.
	if (iter_count == MAX_ITER) {
	//I like red.
	  my_app->pixels[WIDTH*y+x] = 0xFF0000;
	}else{
	//Proportional colouring
	  my_app->pixels[WIDTH*y+x] =  0xFFFFFF/MAX_ITER * iter_count;
	}
      }
   }
   return 0;
}

void change_viewport_wrt_mouse(int mouse_x,int mouse_y,long double offset_x, long double offset_y){
    long double mouse_x_mapped = out_min_x + (out_max_y-out_min_y)/(HEIGHT)*mouse_x;  
    long double mouse_y_mapped = out_min_y + (out_max_y-out_min_y)/(HEIGHT)*mouse_y;  
    out_min_x = mouse_x_mapped - offset_x;
    out_max_x = mouse_x_mapped + offset_x;
    out_min_y = mouse_y_mapped - offset_y;
    out_max_y = mouse_y_mapped + offset_y;
  
  }

int handle_key_presses(int keycode,long double offset_x, long double offset_y,int mouse_x,int mouse_y){
   switch (keycode)
     {
     case SDLK_w:
       //Move up
       //Since y axis is inverted subtracting will take us to upper part of screen
       printf("\r%-110s","Moving up. Wait for image to render!");
       out_min_y -= offset_y/4;
       out_max_y -= offset_y/4;
       break;
     case SDLK_s:
       //Move down
       printf("\r%-110s","Moving down. Wait for image to render!");
       out_min_y += offset_y/4;
       out_max_y += offset_y/4;
       break;
     case SDLK_a:
       //Move left
       printf("\r%-110s","Moving Left. Wait for image to render!");
       out_min_x -= offset_x/4;
       out_max_x -= offset_x/4;
       break;
     case SDLK_d:
       //Move right
       printf("\r%-110s","Moving Right. Wait for image to render!");
       out_min_x += offset_x/4;
       out_max_x += offset_x/4;
       break;
     case SDLK_SPACE:
       //Zoom in
       printf("\r%-110s","Zooming in on mouse pointer. Wait for image to render!");
       change_viewport_wrt_mouse(mouse_x,mouse_y,offset_x/4,offset_y/4);
       MAX_ITER += 20;
       break;
     case SDLK_f:
       //Zoom forever
       printf("\r%-110s","Zooming forever on first mouse pointer location. Wait for image to render!");
       zoom_forever = 1;
       //Center the point under mouse pointer.
       change_viewport_wrt_mouse(mouse_x,mouse_y,offset_x/2,offset_y/2);
       break;
     case SDLK_g:
       //Stop Zoom forever
       printf("\r%-110s","Zooming forever stopped!");
       zoom_forever = 0;
       break;
     case SDLK_c:
       //Center the point under the mouse pointer.
       printf("\r%-110s","Centering the point under mouse pointer. Wait for image to render!");
       change_viewport_wrt_mouse(mouse_x,mouse_y,offset_x/2,offset_y/2);
       break;
     default:
       return 0;
     }
   return 1;
}

void destroy_app(App *my_app){
  free(my_app->pixels);
  SDL_DestroyTexture(my_app->texture);
  SDL_DestroyRenderer(my_app->renderer);
  SDL_DestroyWindow(my_app->window);      
  SDL_Quit();
}
