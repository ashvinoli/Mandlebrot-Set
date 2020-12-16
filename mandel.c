#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>


#define WIDTH 1200
#define HEIGHT 650
#define MAX_ITER 50


void draw(SDL_Renderer **,int);

int main(int argc, char *argv[])
{
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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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
    int factor = 1;

    while (!quit){
	while (SDL_PollEvent(&event))
	{
	    if (event.type == SDL_QUIT)
		quit = 1;
	}

	//Clear using white color
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	//Draw pixels on the renderer
	draw(&renderer,factor);
	SDL_RenderPresent(renderer);

	//Increaseing Factor by 10 each time.
	factor+=1;

    }

    //free resources
    if (renderer){
      SDL_DestroyRenderer(renderer);
    }
    if (window) {
      SDL_DestroyWindow(window);      
    }

    SDL_Quit();
    return 0;
}

void draw(SDL_Renderer **renderer,int factor){
   for (int x = 0; x < WIDTH; x++) {
      for (int y =0;  y < HEIGHT; y++) {
      //Transforming and scaling such that origin is center and  radius of 2 around it. Scaling uniformly for both. Y is still inverted.
	float c_real = (x - WIDTH/2.0) * (4.0/WIDTH); 
	float c_img = (y-HEIGHT/2.0) * (4.0/WIDTH); 

	float z_real = 0;
	float z_img = 0;
	int iter_count = 0;
	while (pow(z_real,2)+pow(z_img,2) <= 4 && iter_count < MAX_ITER) {
	  float temp_real = pow(z_real,2)-pow(z_img,2)+c_real;
	  float temp_img = 2*z_real*z_img + c_img;
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
	  SDL_SetRenderDrawColor(*renderer, iter_count*factor*3,iter_count*factor, iter_count*factor, SDL_ALPHA_OPAQUE);
	  SDL_RenderDrawPoint(*renderer,x,y);
	}
      }
   }
}
