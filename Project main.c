/////////////////////////////////////////////////////////
// Game Programming All In One, Third Edition
// Chapter 16 - PlatformScroller
/////////////////////////////////////////////////////////

#include <stdio.h>
#include <allegro.h>
#include <time.h>
#include <stdlib.h>
#include "mappyal.h"

#define MODE GFX_SAFE
#define WIDTH 640
#define HEIGHT 480
#define JUMPIT 1600

//define the sprite structure
typedef struct SPRITE
{
    int dir, alive;
    int x,y;
    int width,height;
    int xspeed,yspeed;
    int xdelay,ydelay;
    int xcount,ycount;
    int curframe,maxframe,animdir;
    int framecount,framedelay;
}SPRITE;

//declare the bitmaps and sprites
BITMAP *player_image[3];
SPRITE *player;
BITMAP *buffer;	
BITMAP *temp;
BITMAP *bone;
BITMAP *win; 
BITMAP *lose;
BITMAP *intro; 
MIDI *music;
SAMPLE *sample;
int pos, length;
int panning = 128;
int pitch = 1000;
int volume = 500;
int total = 0;
int winlose; 
int level=1; 
//tile grabber
BITMAP *grabframe(BITMAP *source, 
                  int width, int height, 
                  int startx, int starty, 
                  int columns, int frame)
{
    BITMAP *temp = create_bitmap(width,height);
    int x = startx + (frame % columns) * width;
    int y = starty + (frame / columns) * height;
    blit(source,temp,x,y,0,0,width,height);
    return temp;
}


int collided(int x, int y)
{
    BLKSTR *blockdata;

	// Dr. Byun: If y is out of bound, always return 0 (= false)
	if (y < 0)
		return 0;
	blockdata = MapGetBlock(x/mapblockwidth, y/mapblockheight);
	return blockdata->tl;
}

void midiPlayUpdate()
{
	pos = midi_time;
	rest(1); 
}


int main (void)
{
    int mapxoff, mapyoff;
    int oldpy, oldpx;
    int facing = 0;
    int jump = JUMPIT;
    int n;
	int secs = 240;
	double daTime;
	double newTime;
	daTime = clock();

	allegro_init();	
	install_timer();
	install_keyboard();
	set_color_depth(16);
	set_gfx_mode(MODE, WIDTH, HEIGHT, 0, 0);

    temp = load_bitmap("dog.bmp", NULL);
	bone = load_bitmap("bone.bmp", NULL);
	win = load_bitmap("youwin.bmp", NULL);
	lose = load_bitmap("youlose.bmp", NULL);
	intro = load_bitmap("intro.bmp", NULL);
    for (n=0; n<4; n++)
        player_image[n] = grabframe(temp,50,35,0,0,2,n);
	
    destroy_bitmap(temp);

    player = malloc(sizeof(SPRITE));
    player->x = 80;
    player->y = 100;
    player->curframe=0;
    player->framecount=0;
    player->framedelay=1;
    player->maxframe=1;
    player->width=player_image[0]->w;
    player->height=player_image[0]->h;

    //load the map
	MapLoad("level1.fmp"); 

    //create the double buffer
	buffer = create_bitmap (WIDTH, HEIGHT);
	clear(buffer);
	if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0) 
    {
        allegro_message("Error initializing sound system\n%s\n", allegro_error);
        return 1;
    }
	music = load_midi("music.mid");
	sample = load_sample("clapping.wav");
    if (!music) 
    {
        allegro_message("Error loading Midi file");
        return 1;
    }
	if (!sample) 
    {
        allegro_message("Error reading wave file");
        return 1;
    }
	length = get_midi_length(music);
    if (play_midi(music, 1) != 0) 
    {
        allegro_message("Error playing Midi\n%s", allegro_error);
        return 1;
    }
	while(!key[KEY_SPACE])
	{
		blit(intro, screen, 0,0,0,0,WIDTH-1, HEIGHT-1); 
	}
    //main loop
	while (!key[KEY_ESC])
	{
		oldpy = player->y; 
        oldpx = player->x;
		newTime = clock();

		
		if (((newTime - daTime) / CLOCKS_PER_SEC) >=1)
		{
			if((player->x > 15986) && (player->x < 31724) && player->y > 410)
			{
				secs-=5; 
			}
			if((player->x > 31725) && (player->x < 47500) && (player->y > 410))
			{
				secs-=10; 
			}
				secs--;
				daTime = newTime;
			
		}
		if (secs <= 0)
		{
			winlose = 0;
			break;
		}
		if (key[KEY_RIGHT]) 
        { 
            facing = 1; 
            player->x+=2; 
            if (++player->framecount > player->framedelay)
            {
                player->framecount=0;
                if (++player->curframe > player->maxframe)
                    player->curframe=0;
            }
        }
        else if (key[KEY_LEFT]) 
        { 
            facing = 0; 
            player->x-=2; 
            if (++player->framecount > player->framedelay)
            {
                player->framecount=0;
                if (++player->curframe > player->maxframe)
                    player->curframe=0;
            }
        }
        else player->curframe=0;

        //handle jumping
        if (jump==JUMPIT)
        { 
            if (!collided(player->x + player->width/2, 
                player->y + player->height + 5))
                jump = 0; 

		    if (key[KEY_SPACE]) 
                jump = 30;
        }
        else
        {
            player->y -= jump/3; 
            jump--; 
        }

		if (jump<0) 
        { 
            if (collided(player->x + player->width/2, 
                player->y + player->height))
			{ 
                jump = JUMPIT; 
                while (collided(player->x + player->width/2, 
                    player->y + player->height))
                    player->y -= 2; 
            } 
        }

        //check for collided with foreground tiles
		if (!facing) 
        { 
            if (collided(player->x, player->y + player->height)) 
                player->x = oldpx; 
        }
		else 
        { 
            if (collided(player->x + player->width, 
                player->y + player->height)) 
                player->x = oldpx; 
        }
		
        //update the map scroll position
		mapxoff = player->x + player->width/2 - WIDTH/2 + 10;
		mapyoff = player->y + player->height/2 - HEIGHT/2 + 10;

		if(player->x == 15986)
		{
			level=2;
			total+=secs;
			secs = 260;
			play_sample(sample, volume, panning, pitch, FALSE);
		}
		if(player->x == 31724)
		{
			level=3; 
			total+=secs;
			secs = 130;
			play_sample(sample, volume, panning, pitch, FALSE);
		}
		if(player->x == 37018)
		{
			total+=secs;
			secs = 140; 
			play_sample(sample, volume, panning, pitch, FALSE);
		}
		if(player->x == 42992)
		{
			total+=secs;
			secs = 140; 
			play_sample(sample, volume, panning, pitch, FALSE);
		}
		if(player->x == 47764)
		{
			winlose = 1; 
			break; 
		}
		

        //avoid moving beyond the map edge
		if (mapxoff < 0) mapxoff = 0;
		if (mapxoff > (mapwidth * mapblockwidth - WIDTH))
            mapxoff = mapwidth * mapblockwidth - WIDTH;
		if (mapyoff < 0) 
            mapyoff = 0;
		if (mapyoff > (mapheight * mapblockheight - HEIGHT)) 
            mapyoff = mapheight * mapblockheight - HEIGHT;

        //draw the background tiles
		MapDrawBG(buffer, mapxoff, mapyoff, 0, 0, WIDTH-1, HEIGHT-1);

        //draw foreground tiles
		MapDrawFG(buffer, mapxoff, mapyoff, 0, 0, WIDTH-1, HEIGHT-1, 0);
        //draw the player's sprite
		if (facing) 
            draw_sprite(buffer, player_image[player->curframe], 
                (player->x-mapxoff), (player->y-mapyoff+1));
		else 
            draw_sprite_h_flip(buffer, player_image[player->curframe], 
                (player->x-mapxoff), (player->y-mapyoff));

		

        textprintf_ex(buffer,font,0,0,-1,0,
            "TIME LEFT = %d, Level %d", secs, level);
        //blit the double buffer 
		midiPlayUpdate(); 
		rest(5); 
        acquire_screen();
		blit(buffer, screen, 0, 0, 0, 0, WIDTH-1, HEIGHT-1);
        release_screen();

	} //while

	while(!key[KEY_ESC])
	{
		if(winlose == 1)
		{
			blit(win, screen, 0,0,0,0,WIDTH-1, HEIGHT-1); 
		}
		else if(winlose == 0)
		{
			blit(lose, screen, 0,0,0,0,WIDTH-1, HEIGHT-1); 
			stop_midi(); 
		}
	}

    for (n=0; n<2; n++)
        destroy_bitmap(player_image[n]);
    free(player);
	stop_midi(); 
	destroy_midi(music); 
	destroy_bitmap(buffer);
	MapFreeMem ();
	allegro_exit();
	return 0;
}
END_OF_MAIN()
