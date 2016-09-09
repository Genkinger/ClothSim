#include <stdio.h>
#include <stdlib.h>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <memory>

#include <random>
#include <functional>
#include <chrono>

#define WIDTH 800
#define HEIGHT 600

void parseCFG(char* path)
{
		
}

float getLength(float x0, float y0, float x1, float y1) 
{
	float dx = x0 - x1;
	float dy = y0 - y1;
	float l = sqrtf(dx * dx + dy * dy);
	if (l < 0) 
	{
		return -l;
	}
	else 
	{
		return l;
	}
}

bool inRange(float value, float min, float max) 
{
	if (value > min && value < max) 
	{
		return true;
	}
	else 
	{
		return false;
	}
}

struct V_Point {
	float x;
	float y;
	float oldx;
	float oldy;
	bool pinned;
};

struct V_Stick {
	V_Stick(V_Point* pA, V_Point* pB) : p0(pA), p1(pB) {}
	V_Point *p0;
	V_Point *p1;
	float length;
	float tear_length;
	bool dead;
};

void updatePoints(V_Point* points[], long long size,float delta)
{
	float gravity = 0.7;
	float friction = 0.98;

	for (long long i = 0; i < size; i++)
	{
		if (!points[i]->pinned) {

			float vx = (points[i]->x - points[i]->oldx) * friction ;
			float vy = (points[i]->y - points[i]->oldy) * friction ;

			points[i]->oldx = points[i]->x;
			points[i]->oldy = points[i]->y;

			points[i]->x += vx;
			points[i]->y += vy;
			points[i]->y += gravity;
		}
	}
}

void updateSticks(V_Stick* sticks[], long long size,float delta) 
{
	for (int i = 0; i < size; i++)
	{
		
		float dx = sticks[i]->p1->x - sticks[i]->p0->x;
		float dy = sticks[i]->p1->y - sticks[i]->p0->y;
		float distance = sqrtf(dx*dx + dy*dy);

		if (distance > sticks[i]->tear_length) 
		{
			sticks[i]->dead = true;
		}

		float diff = sticks[i]->length - distance;
		float percent = diff / distance / 2;
		float offsetX = dx * percent;
		float offsetY = dy * percent;

		if (!sticks[i]->dead) {

			if (!sticks[i]->p0->pinned) {
				sticks[i]->p0->x -= offsetX;
				sticks[i]->p0->y -= offsetY;
			}
			if (!sticks[i]->p1->pinned) {
				sticks[i]->p1->x += offsetX;
				sticks[i]->p1->y += offsetY;
			}
		}
	}
}

void constrainPoints(V_Point* points[], long long size,float delta){
	float friction = 0.98;
	for (long long i = 0; i < size; i++)
	{
		if (!points[i]->pinned) {

			float vx = (points[i]->x - points[i]->oldx) * friction;
			float vy = (points[i]->y - points[i]->oldy) * friction;

			if (points[i]->x > WIDTH)
			{
				points[i]->x = WIDTH;
				points[i]->oldx = points[i]->x + vx;
			}
			else if (points[i]->x < 0)
			{
				points[i]->x = 0;
				points[i]->oldx = points[i]->x + vx;
			}

			if (points[i]->y > HEIGHT)
			{
				points[i]->y = HEIGHT;
				points[i]->oldy = points[i]->y + vy;
			}
			else if (points[i]->y < 0)
			{
				points[i]->y = 0;
				points[i]->oldy = points[i]->y + vy;
			}
		}
	}
}

void drawPoints(V_Point* points[], long long size, sf::RenderWindow& window) 
{

	sf::Vertex* arr = new sf::Vertex[6 * size];
	int sz = 3;
	int c = 0;
	for (long long i = 0; i < size; i++) 
	{
		arr[c++] = sf::Vertex(sf::Vector2f(points[i]->x - sz, points[i]->y + sz),sf::Color::Black);
		arr[c++] = sf::Vertex(sf::Vector2f(points[i]->x - sz, points[i]->y - sz), sf::Color::Black);
		arr[c++] = sf::Vertex(sf::Vector2f(points[i]->x + sz, points[i]->y - sz), sf::Color::Black);
		arr[c++] = sf::Vertex(sf::Vector2f(points[i]->x + sz, points[i]->y - sz), sf::Color::Black);
		arr[c++] = sf::Vertex(sf::Vector2f(points[i]->x + sz, points[i]->y + sz), sf::Color::Black);
		arr[c++] = sf::Vertex(sf::Vector2f(points[i]->x - sz, points[i]->y + sz), sf::Color::Black);
		
	}
		window.draw(arr,size * 6,sf::Triangles);


	delete[] arr;

}

void drawSticks(V_Stick* sticks[],long long size, sf::RenderWindow& window)
{
	sf::Vertex* lines = new sf::Vertex[size * 2];
	int c = 0;

	for (int i = 0; i < size; i++)
	{
		if(!sticks[i]->dead){
			lines[c++] = sf::Vertex(sf::Vector2f(sticks[i]->p0->x, sticks[i]->p0->y), sf::Color::Red);
			lines[c++] = sf::Vertex(sf::Vector2f(sticks[i]->p1->x, sticks[i]->p1->y), sf::Color::Red);
		}
	}
	window.draw(lines, 2 * size, sf::Lines);

	delete[] lines;
}

void init(V_Point** pts, V_Stick** sticks, int rows, int cols) 
{
	std::uniform_int_distribution<int> distrib(0,10);
	std::mt19937 gen;

	int i = 0;
	int nP = 0;
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			V_Point* point = new V_Point;
			point->x = col * WIDTH / cols + 1;
			point->y = row * HEIGHT / cols + 10;
			point->oldx = point->x;
			point->oldy = point->y;
			point->pinned = false;



			if (col != 0)
			{
				V_Stick* stick = new V_Stick(point, pts[nP - 1]);
				stick->dead = false;
				stick->length = getLength(point->x, point->y, pts[nP - 1]->x, pts[nP - 1]->y);
				stick->tear_length = stick->length + 30  + distrib(gen);
				sticks[i++] = stick;
			}

			if (row != 0 && row != 1)
			{
				V_Stick* stick = new V_Stick(point, pts[(row - 1) * cols + col]);
				stick->dead = false;
				stick->length = getLength(point->x, point->y, pts[(row - 1) * cols + col]->x, pts[(row - 1) * cols + col]->y);
				stick->tear_length = stick->length + 30 + distrib(gen);
				sticks[i++] = stick;
			}
			if (row == 1)
			{
				V_Stick* stick = new V_Stick(point, pts[(row - 1) * cols + col]);
				stick->dead = false;
				stick->length = getLength(point->x, point->y, pts[(row - 1) * cols + col]->x, pts[(row - 1) * cols + col]->y);
				stick->tear_length = stick->length + 300;
				sticks[i++] = stick;
			}

			if (row == 0)
			{
				point->pinned = true;
			}

			pts[nP++] = point;
		}
	}
}




int main()
{
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Verlet Test");
	window.setFramerateLimit(60);
	sf::Event ev;
	sf::Clock clock;
	sf::Time dt;
	int rows = 60;
	int cols = 150;

	V_Point** pts = new V_Point*[rows * cols];

	V_Stick** sticks = new V_Stick*[rows * cols * 2 - (rows + cols)];

	init(pts, sticks, rows, cols);


	while (window.isOpen())
	{
		window.pollEvent(ev);
		if (ev.type == sf::Event::Closed)
		{
			
			window.close();
		}

		window.clear(sf::Color(220, 220, 220));
	
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) 
		{
			sf::Vector2i pos = sf::Mouse::getPosition(window);
			for (int i = 0; i < rows * cols; i++)
			{
				if (!pts[i]->pinned) {
				float dist = getLength(pts[i]->x, pts[i]->y, pos.x, pos.y);

				if (inRange(dist,0,30) )
				{
					pts[i]->x -= 20;
					pts[i]->y += 20;
					pts[i]->oldx = pts[i]->x;
					pts[i]->oldy = pts[i]->y;
				}
				else if (inRange(dist, -30, 0))
				{
					pts[i]->x -= 20;
					pts[i]->y += 20;
					pts[i]->oldx = pts[i]->x;
					pts[i]->oldy = pts[i]->y;
				}
				}
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) 
		{
			for (int i = 0; i < rows * cols; i++)
			{
				delete (V_Point*)pts[i];
			}

			for (int i = 0; i < rows * cols * 2 - (rows + cols); i++)
			{
				delete (V_Stick*)sticks[i];
			}

			delete[] sticks;
			delete[] pts;

		
			pts = new V_Point*[rows * cols];
			sticks = new V_Stick*[rows * cols * 2 - (rows + cols)];
			
			init(pts, sticks, rows, cols);
		}

		updatePoints(pts, rows * cols, dt.asSeconds());
		
		for (int i = 0; i < 10; i++) 
		{
			updateSticks(sticks, rows * cols * 2 - (rows + cols),dt.asSeconds());
			constrainPoints(pts, rows * cols, dt.asSeconds());
		}

		
		drawSticks(sticks, rows * cols * 2- (rows + cols), window);
		//drawPoints(pts, 120,window);
		window.display();
		dt = clock.restart();

	}

	return EXIT_SUCCESS;
}
