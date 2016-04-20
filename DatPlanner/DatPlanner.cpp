// DatPlanner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef _DEBUG
	#pragma comment(lib, "sfml-system-d.lib")
	#pragma comment(lib, "sfml-main-d.lib")
	#pragma comment(lib, "sfml-graphics-d.lib")
	#pragma comment(lib, "sfml-window-d.lib")

#else
	#pragma comment(lib, "sfml-system.lib")
	#pragma comment(lib, "sfml-main.lib")
	#pragma comment(lib, "sfml-graphics.lib")
	#pragma comment(lib, "sfml-window.lib")
#endif

std::stringstream ss;

struct Day
{
	int year;
	int month;
	int day;
};

bool operator==(Day& d1, Day& d2)
{
	return d1.year == d2.year && d1.month == d2.month && d1.day == d2.day;
}

struct RecurrentEvent
{
	int year;
	int month;
	int week;
	int day;
	std::time_t fist_occurence;
	int day_minutes_start;
	int day_minutes_end;
};

struct SingleEvent
{
	Day day;
	int day_minutes_start;
	int day_minutes_end;
};

struct
{
	bool button_pweek_hover;
	bool button_nweek_hover;
	bool button_pweek_push;
	bool button_nweek_push;

	float start_minute;
	float show_minutes;

	float xmargin;
	float ymargin;

	float dy;
	float dx;

	bool right_mouse_button_down;
	float drag_start_minute;
	float drag_start_y;

	std::list<RecurrentEvent> recurrent_events;

	std::list<SingleEvent> next_single_events;
	std::list<SingleEvent> past_previous_events;

} calendar;

void display_week(sf::RenderWindow& window, sf::Text& text, int deltaweek)
{
	int start_hour = (int)calendar.start_minute / 60;
	
	Day current_week_days[7];

	static const std::string weekdays[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
	static const std::string months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	std::time_t t = std::time(nullptr);
	std::tm *tm = std::localtime(&t);

	int first_day = tm->tm_mday - tm->tm_wday;
	int week_day = tm->tm_wday;

	for (int i = 0; i < 7; i++)
	{
		int delta_day = i - week_day + 1;
		std::time_t day_t = std::time(nullptr) + delta_day * 3600 * 24 + deltaweek * 3600 * 24 * 7;

		tm = std::localtime(&day_t);

		current_week_days[i] = { tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday };
	}

	int current_day = week_day - 1;


	text.setString("00");
	float xmargin = text.getLocalBounds().width + text.getLocalBounds().left*2;
	calendar.xmargin = xmargin;

	float dx = (window.getSize().x  - 2*xmargin) / 7;

	sf::RectangleShape rshape;

	float text_height = 0;

	text.setColor(sf::Color::White);

	t = std::time(nullptr);
	tm = std::localtime(&t);

	// Draw columns
	for (int i = 0; i < 7; i++)
	{
		// This is our day
		if ( current_week_days[i].day == tm->tm_mday)
		{
			sf::RectangleShape shape;
			shape.setFillColor(sf::Color(50, 50, 50, 255));
			shape.setPosition(xmargin + dx*i + 1, 0);
			shape.setSize(sf::Vector2f{ (float)dx - 2, (float)window.getSize().y });
			window.draw(shape);
		}

		ss.str("");
		ss << weekdays[i] << " - " << current_week_days[i].day;
		text.setString(ss.str());
		sf::FloatRect rect = text.getLocalBounds();
		float x = xmargin + dx*i + dx / 2 - rect.width / 2;
		text_height = rect.height + rect.top + 1;
		
		text.setPosition(x, 0);
		
		window.draw(text);
		
		if (i > 0)
		{
			rshape.setPosition(xmargin + dx*i, 0);
			rshape.setSize({ 1,(float)window.getSize().y });
			window.draw(rshape);
		}
	}

	// Draw margins
	rshape.setSize({ 1, (float)window.getSize().y});
	rshape.setPosition(xmargin, 0);
	window.draw(rshape);

	rshape.setSize({ 1, (float)window.getSize().y });
	rshape.setPosition(window.getSize().x - xmargin, 0);
	window.draw(rshape);

	// Draw top margin
	rshape.setSize({ (float)window.getSize().x, 2 });
	rshape.setPosition(0, text_height);
	window.draw(rshape);

	// Draw rows
	float dy = (window.getSize().y - text_height) / (calendar.show_minutes/60);

	text.setColor(sf::Color(128, 128, 128));

	for (int i = 0; i <= 24; i++)
	{
		float y = text_height + i*dy - calendar.start_minute*dy / 60;


		if (y > text_height)
		{
			rshape.setSize({window.getSize().x - 2*xmargin, 1.0f });
			rshape.setPosition(xmargin, y);
			rshape.setFillColor(sf::Color(128, 128, 128));
			window.draw(rshape);
		}

		ss.str("");
		ss << std::setfill('0') << std::setw(2);
		ss << i;
		text.setString(ss.str());
		text.setPosition(0, y -  text_height/2);

		
		if (text.getGlobalBounds().top < -dy + text_height + text.getLocalBounds().height)
		{
			continue;
		}
		else if (text.getGlobalBounds().top < text_height + 3)
		{
			text.setPosition(text.getPosition().x, text.getPosition().y + (text_height - text.getGlobalBounds().top) + 3);
			window.draw(text);
		}
		else if (text.getPosition().y + (text.getLocalBounds().top + text.getLocalBounds().height) + text.getLocalBounds().height > window.getSize().y + dy)
		{
			continue;
		}
		else if (text.getPosition().y + (text.getLocalBounds().top + text.getLocalBounds().height + 3) > window.getSize().y)
		{
			text.setPosition(text.getPosition().x, window.getSize().y - (text.getLocalBounds().top + text.getLocalBounds().height + 3));
			window.draw(text);
		}
		
		text.setPosition(0, text.getPosition().y);
		window.draw(text);
		
		text.setPosition(window.getSize().x - (text.getLocalBounds().width + text.getLocalBounds().left), text.getPosition().y);
		window.draw(text);
	}

	// Draw current time
	if (deltaweek == 0)
	{
		rshape.setFillColor(sf::Color::Green);
		rshape.setSize({ (float)dx -1, 1 });
		rshape.setPosition(xmargin + dx*current_day, (tm->tm_min + tm->tm_hour * 60 - calendar.start_minute)*dy / 60 + text_height);
		if (rshape.getPosition().y > text_height)
			window.draw(rshape);
	}

	// Draw events
	int i = 0;
	for (auto& single_event : calendar.next_single_events)
	{
		i = 0;
		for (auto& day : current_week_days)
		{
			if (day == single_event.day)
			{
				rshape.setFillColor(sf::Color::Green);
				rshape.setSize({(dx - 3), (single_event.day_minutes_end - single_event.day_minutes_start)*dy/60 - 2});
				rshape.setPosition(xmargin + dx*i + 2, (single_event.day_minutes_start - calendar.start_minute)*dy / 60 + text_height + 2);
				if (rshape.getPosition().y <= text_height && rshape.getGlobalBounds().top + rshape.getSize().y > text_height)
				{
					rshape.setSize({ rshape.getSize().x, rshape.getSize().y - (text_height - rshape.getPosition().y)  - 2 });
					rshape.setPosition(rshape.getPosition().x, text_height + 2);
					window.draw(rshape);
				}
				else if (rshape.getPosition().y > text_height)
					window.draw(rshape);
			}

			i++;
		}
	}
	


	// Draw week buttons
	rshape.setSize({ (float)xmargin, (float)text_height });
	rshape.setPosition(0, 0);
	if (calendar.button_pweek_push)
	{
		rshape.setFillColor(sf::Color(128, 128, 128));
	} 
	else if (calendar.button_pweek_hover)
	{
		rshape.setFillColor(sf::Color(64, 64, 64));
	}
	else
	{
		rshape.setFillColor(sf::Color(0, 0, 0));
	}

	window.draw(rshape);

	text.setColor(sf::Color::White);
	text.setString("<");
	text.setPosition(0, 0);
	text.setPosition(-(text.getLocalBounds().left + text.getLocalBounds().width / 2) + xmargin/2, -(text.getLocalBounds().top + text.getLocalBounds().height/2) + text_height/2);
	window.draw(text);

	rshape.setPosition(window.getSize().x - xmargin + 1, 0);
	if (calendar.button_nweek_push)
	{
		rshape.setFillColor(sf::Color(128, 128, 128));
	}
	else if (calendar.button_nweek_hover)
	{
		rshape.setFillColor(sf::Color(64, 64, 64));
	}
	else
	{
		rshape.setFillColor(sf::Color(0, 0, 0));
	}

	window.draw(rshape);

	text.setString(">");
	text.setPosition(0, 0);
	text.setPosition(-(text.getLocalBounds().left + text.getLocalBounds().width / 2) + window.getSize().x - xmargin / 2, -(text.getLocalBounds().top + text.getLocalBounds().height / 2) + text_height / 2);
	window.draw(text);

	calendar.ymargin = text_height;
	calendar.dy = dy;
}

int main(int argc, char **argv)
{
	//calendar.recurrent_events.push_back({0,0,});
	calendar.start_minute = 7 * 60 + 45;
	calendar.show_minutes = 11 * 60 + 30;

	calendar.next_single_events.push_back({ { 2016, 4, 21 }, 8 * 60, 12 * 60 });
	calendar.next_single_events.push_back({ { 2016, 4, 23 }, 8 * 60, 12 * 60 });
	calendar.next_single_events.push_back({ { 2016, 4, 18 }, 10 * 60, 12 * 60 });
	calendar.next_single_events.push_back({ { 2016, 4, 18 }, 14 * 60, 19 * 60 });

	bool mouse_right_button_down = false;
	int mouse_drag_x_start = 0;
	int mouse_drag_y_start = 0;

	int deltaweek = 0;
	auto now = std::chrono::system_clock::now();
	auto now_t = std::chrono::system_clock::to_time_t(now);
	
	std::time_t t = std::time(nullptr);
	std::tm *tm = std::localtime(&t);

	cout << std::ctime(&now_t) << endl;

	cout << "Week " << tm->tm_yday / 7 << endl;

	sf::RenderWindow window;
	
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	window.create(sf::VideoMode(1280, 720), "Dat Planner", sf::Style::Default, settings);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(30);

	sf::Font font;
	bool found_font = font.loadFromFile("C:/windows/fonts/courbd.ttf");
	
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(20);

	while (window.isOpen())
	{
		window.clear(sf::Color::Black);
		display_week(window, text, deltaweek);
		window.display();

		sf::Event event;

		window.waitEvent(event);
		do
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;

			case sf::Event::Resized:
				window.setView(sf::View({ 0,0,(float)event.size.width, (float)event.size.height }));
				cout << "resize: " << event.size.width << ", " << event.size.height << endl;
				break;

			case sf::Event::MouseWheelScrolled:
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
				{
					calendar.show_minutes -= (int)event.mouseWheelScroll.delta * 20;
				}
				else
					calendar.start_minute -= (int)event.mouseWheelScroll.delta*5;
				break;

			case sf::Event::KeyPressed:
				switch (event.key.code)
				{
				case sf::Keyboard::Up:
					calendar.start_minute += 1;
					break;
				case sf::Keyboard::Down:
					calendar.start_minute -= 1;
					
					break;

				case sf::Keyboard::N: // Now
					deltaweek = 0;
					std::time_t t = std::time(nullptr);
					std::tm *tm = std::localtime(&t);

					calendar.start_minute = tm->tm_min + tm->tm_hour * 60 - calendar.show_minutes / 2;
					break;
				}
				break;
			case sf::Event::MouseMoved:
				calendar.button_pweek_hover = false;
				calendar.button_nweek_hover = false;

				if (event.mouseMove.x < calendar.xmargin && event.mouseMove.y < calendar.ymargin)
				{
					calendar.button_pweek_hover = true;
				}

				if (event.mouseMove.x > window.getSize().x - calendar.xmargin && event.mouseMove.y < calendar.ymargin)
				{
					calendar.button_nweek_hover = true;
				}

				// We are dragging
				if (calendar.right_mouse_button_down)
				{
					float delta_y = event.mouseMove.y - calendar.drag_start_y;
					calendar.start_minute = calendar.drag_start_minute - delta_y * 60.0f / calendar.dy;
				}

				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Button::Left)
				{
					calendar.button_pweek_push = false;
					calendar.button_nweek_push = false;

					if (calendar.button_pweek_hover)
					{
						calendar.button_pweek_push = true;
					}

					if (calendar.button_nweek_hover)
					{
						calendar.button_nweek_push = true;
					}
				}
				
				if (event.mouseButton.button == sf::Mouse::Button::Right)
				{
					calendar.right_mouse_button_down = true;
					calendar.drag_start_y = (float)event.mouseButton.y;
					calendar.drag_start_minute = calendar.start_minute;
				}
				break;

			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Button::Left)
				{
					if (calendar.button_pweek_push && calendar.button_pweek_hover)
					{
						deltaweek -= 1;
					}

					if (calendar.button_nweek_push && calendar.button_nweek_hover)
					{
						deltaweek += 1;
					}
					calendar.button_pweek_push = false;
					calendar.button_nweek_push = false;
				}

				if (event.mouseButton.button == sf::Mouse::Button::Right)
				{
					calendar.right_mouse_button_down = false;
				}
				break;

			}

			if (calendar.show_minutes > 24 * 60)
			{
				calendar.show_minutes = 24 * 60;
			}

			if (calendar.start_minute < 0)
				calendar.start_minute = 0;
			if (calendar.start_minute >(24 * 60 - calendar.show_minutes))
				calendar.start_minute = 24 * 60 - calendar.show_minutes;

			

			//cout << "start minute: " << start_minute << endl;

		} while (window.pollEvent(event));	
	}

    return 0;
}

