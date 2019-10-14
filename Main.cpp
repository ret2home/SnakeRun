#include <bits/stdc++.h>
# include <Siv3D.hpp> // OpenSiv3D v0.4.1
using namespace std;

struct Point2D {
	double x, y;
};
struct ColorPoint2D :Point2D {
	double Color, trans;
};
struct Snake {
	vector<Point2D>pos;
	void update(bool automode, bool isfast, double x, double y) {
		if (!automode && sqrt(pow(abs(x - pos[0].x), 2) + pow(abs(y - pos[0].y), 2)) <= 15.0)goto loop;

		double degree;
		if (!automode)degree = 90 - ToDegrees(atan2(pos[0].y - y, x - pos[0].x));
		else degree = 90 - ToDegrees(atan2(pos[0].y - y, x - pos[0].x));

		if (isfast && !automode) {
			pos[0].x = pos[0].x + cos(ToRadians(90 - degree)) * Scene::DeltaTime() * 360.;
			pos[0].y = pos[0].y - sin(ToRadians(90 - degree)) * Scene::DeltaTime() * 360.;
		}
		else if (isfast) {
			pos[0].x = pos[0].x + cos(ToRadians(90 - degree)) * Scene::DeltaTime() * 180.;
			pos[0].y = pos[0].y - sin(ToRadians(90 - degree)) * Scene::DeltaTime() * 180.;
		}
		else if (!automode) {
			pos[0].x = pos[0].x + cos(ToRadians(90 - degree)) * Scene::DeltaTime() * 120.;
			pos[0].y = pos[0].y - sin(ToRadians(90 - degree)) * Scene::DeltaTime() * 120.;
		}
		else {
			pos[0].x = pos[0].x + cos(ToRadians(90 - degree)) * Scene::DeltaTime() * 90.;
			pos[0].y = pos[0].y - sin(ToRadians(90 - degree)) * Scene::DeltaTime() * 90.;
		}
	loop:;

		for (int i = 1; i < pos.size(); i++) {
			degree = 90 - ToDegrees(atan2(pos[i].y - pos[i - 1].y, pos[i - 1].x - pos[i].x));
			pos[i].x = pos[i - 1].x - cos(ToRadians(90 - degree)) * 14.;
			pos[i].y = pos[i - 1].y + sin(ToRadians(90 - degree)) * 14.;
		}
	}
	void unite(int size) {
		double degree = 0;
		for (int i = 0; i < size; i++) {
			degree = Random(degree - 30, degree + 30);
			double x = pos.back().x + cos(ToRadians(90 - degree)) * 14.;
			double y = pos.back().y - sin(ToRadians(90 - degree)) * 14.;
			pos.push_back({ x,y });
		}
	}
	void init(double x, double y, double size = 3) {
		pos.clear();
		pos.push_back({ x,y });
		unite(size - 1);
	}
};
double dis(double x, double y, double x_, double y_) {
	return sqrt(pow(abs(x - x_), 2) + pow(abs(y - y_), 2));
}
struct Game {
	Snake snake;
	vector<Snake>other;
	Point standard;
	vector<ColorPoint2D>dead;
	vector<Point2D>back;
	bool isdead = false;
	void init(int size = 100) {
		isdead = false;
		other.clear();
		dead.clear();
		snake.init(600, 300, 5);
		for (int i = 0; i < size; i++) {
			Snake s;
			double x, y;
		loop:;
			x = Random(-1000, 2200); y = Random(-1000, 1600);
			if (x >= 400 && x <= 800 && y >= 100 && y <= 500)goto loop;
			s.init(x, y);
			other.push_back(s);
		}
		back.clear();
		for (double i = -100; i < 1300; i += 50) {
			for (double j = -100; j < 700; j += 50) {
				back.push_back({ i ,j });
			}
		}
	}
	bool update() {
		snake.update(false || isdead, MouseL.pressed(), Cursor::Pos().x, Cursor::Pos().y);

		for (int i = 0; i < other.size(); i++) {
			int chindex = -1, esindex = -1;
			double chase = dis(snake.pos.back().x, snake.pos.back().y, other[i].pos[0].x, other[i].pos[0].y);
			double escape = dis(snake.pos[0].x, snake.pos[0].y, other[i].pos.back().x, other[i].pos.back().y);
			if (chase >= escape) {
				chindex = 1; chase = 10000.;
			}
			for (int j = 0; j < other.size(); j++) {
				if (i == j)continue;
				double di1 = dis(other[i].pos[0].x, other[i].pos[0].y, other[j].pos.back().x, other[j].pos.back().y);
				double di2 = dis(other[i].pos.back().x, other[i].pos.back().y, other[j].pos[0].x, other[j].pos[0].y);
				if (escape > di2) {
					escape = di2;
					esindex = j;
				}
				if (chase > di1&& di2 > di1) {
					chindex = j; chase = di1;
				}
			}
			if (chindex == -1) {
				other[i].update(true, chase < 200., snake.pos.back().x, snake.pos.back().y);
			}
			else if (esindex == -1) {
				other[i].update(true, escape < 200., other[chindex].pos.back().x, other[chindex].pos.back().y);
			}
			else {
				other[i].update(true, chase < 200., other[chindex].pos.back().x, other[chindex].pos.back().y);
			}
		}

		Circle tip(snake.pos[0].x, snake.pos[0].y, 15);
		for (int i = 0; i < other.size(); i++) {
			if (tip.intersects(Circle(other[i].pos.back().x, other[i].pos.back().y, 15))) {
				snake.unite(other[i].pos.size());
				for (int j = 0; j < other[i].pos.size(); j++) {
					dead.push_back({ other[i].pos[j].x,other[i].pos[j].y,180. / other[i].pos.size() * j,1. });
				}
				other[i] = other.back(); other.pop_back();
				if (other.size() < 50) {
					Snake s; double x, y;
				loop:;
					x = Random(-1000, 2200); y = Random(-1000, 1600);
					if (x >= 0 && x <= 1200 && y >= 0 && y <= 600)goto loop;
					s.init(x, y);
					other.push_back(s);
				}
				return true;
			}
		}
		for (int i = 0; i < other.size(); i++) {
			Circle c1(other[i].pos[0].x, other[i].pos[0].y, 7);
			for (int j = 0; j < other.size(); j++) {
				if (i == j)continue;
				if (c1.intersects(Circle(other[j].pos.back().x, other[j].pos.back().y, 7))) {
					other[i].unite(other[j].pos.size());
					for (int k = 0; k < other[j].pos.size(); k++) {
						dead.push_back({ other[j].pos[k].x,other[j].pos[k].y,180. / other[j].pos.size() * k,1.0 });
					}
					other[j] = other.back(); other.pop_back();
					if (other.size() < 50) {
						Snake s;
						double x, y;
					loop2:;
						x = Random(-1000, 2200); y = Random(-1000, 1600);
						if (x >= 0 && x <= 1200 && y >= 0 && y <= 600)goto loop2;
						s.init(x, y);
						other.push_back(s);
					}
					return true;
				}
			}
		}
		Circle end(snake.pos.back().x, snake.pos.back().y, 7);
		for (int i = 0; i < other.size(); i++) {
			if (end.intersects(Circle(other[i].pos[0].x, other[i].pos[0].y, 7))) {
				for (int j = 0; j < snake.pos.size(); j++) {
					if (j == 0)dead.push_back({ snake.pos[j].x,snake.pos[j].y,0,1.0 });
					else if (j != snake.pos.size() - 1)dead.push_back({ snake.pos[j].x,snake.pos[j].y,120,1.0 });
					else dead.push_back({ snake.pos[j].x,snake.pos[j].y,60,1.0 });
				}
				isdead = true;
				return false;
			}
		}
		return true;
	}
	void draw(double speed = 0.03) {
		standard.x = 600 - snake.pos[0].x;
		standard.y = 300 - snake.pos[0].y;

		int diffx = (1300 - back.back().x) / 50, diffy = (700 - back.back().y) / 50;
		for (int i = 0; i < back.size(); i++) {
			if (!isdead) {
				back[i].x += standard.x;
				back[i].y += standard.y;
				back[i].x += diffx * 50;
				back[i].y += diffy * 50;
			}
			if (back[i].x >= -100 && back[i].x <= 1300 && back[i].y >= -100 && back[i].y <= 700) {
				Rect(back[i].x, back[i].y, 40, 40).draw(Palette::Black);
			}
		}

		for (int i = 0; i < other.size(); i++) {
			for (int j = 0; j < other[i].pos.size(); j++) {
				if (standard.x + other[i].pos[j].x >= 0 && standard.x + other[i].pos[j].x <= 1200 && standard.y + other[i].pos[j].y >= 0 && standard.y + other[i].pos[j].y <= 600) {
					Circle(standard.x + other[i].pos[j].x, standard.y + other[i].pos[j].y, 7).draw(HSV(180. / other[i].pos.size() * j));
				}
				if (!isdead) {
					other[i].pos[j].x += standard.x; other[i].pos[j].y += standard.y;
				}
			}
		}
		if (!isdead) {
			for (int i = 0; i < snake.pos.size(); i++) {
				if (standard.x + snake.pos[i].x >= 0 && standard.x + snake.pos[i].x <= 1200 && standard.y + snake.pos[i].y >= 0 && standard.y + snake.pos[i].y <= 600) {
					if (i == 0)Circle(standard.x + snake.pos[i].x, standard.y + snake.pos[i].y, 7).draw(HSV(0));
					else if (i != snake.pos.size() - 1)Circle(standard.x + snake.pos[i].x, standard.y + snake.pos[i].y, 7).draw(HSV(120));
					else Circle(standard.x + snake.pos[i].x, standard.y + snake.pos[i].y, 7).draw(HSV(60));
				}
				snake.pos[i].x += standard.x;
				snake.pos[i].y += standard.y;
			}
		}
		for (int i = 0; i < dead.size(); i++) {
			if (standard.x + dead[i].x >= 0 && standard.x + dead[i].x <= 1200 && standard.y + dead[i].y >= 0 && standard.y + dead[i].y <= 600) {
				Circle(standard.x + dead[i].x, standard.y + dead[i].y, 7).draw(HSV(dead[i].Color, dead[i].trans));
			}
			if (!isdead) {
				dead[i].x += standard.x;
				dead[i].y += standard.y;
			}
			dead[i].trans -= speed;
			if (dead[i].trans == 0) {
				dead.erase(dead.begin() + i); i--;
			}
		}
	}
};
void Main() {
	Window::Resize(1200, 600);
	Window::SetTitle(U"Snake Run");

	Game game;
	int situation = 0;
	bool got_scores = false, wrote_score = false;
	vector<int>scores;
	Font font_50(50, Typeface::Regular, FontStyle::Italic), font_120(120, Typeface::Regular, FontStyle::Italic), font_35(35, Typeface::Regular, FontStyle::Italic), font_75(75, Typeface::Regular, FontStyle::Italic);
	Font font_25(25, Typeface::Regular, FontStyle::Italic);
	Stopwatch gamingtime;

	int score = 0;
	while (System::Update()) {
		if (situation == 0) {
			String title = U"SnakeRun";
			for (int i = 0; i < title.size(); i++) {
				font_120(title[i]).draw(320 + i * 70, 60, HSV(i * 36));
			}
			Rect(420, 270, 360, 60).shearedX(120).draw(Palette::Turquoise);
			Rect(420, 360, 360, 60).shearedX(120).draw(Palette::Turquoise);
			font_35(U"遊び方").drawAt(600, 300);
			font_35(U"プレイ").drawAt(600, 390);
			for (int i = 1; i <= 2; i++) {
				if (Rect(420, 180 + 90 * i, 360, 60).shearedX(120).mouseOver()) {
					Rect(420, 180 + 90 * i, 360, 60).shearedX(120).drawFrame(7, Palette::Yellow);
					if (Rect(420, 180 + 90 * i, 360, 60).shearedX(120).leftClicked()) {
						situation = i;
						if (i == 2) {
							game.init(); gamingtime.start();
						}
					}
				}
			}
			if (!got_scores) {
				ifstream fin; fin.open("GameData/Scores.txt", ios::in);
				while (!fin.eof()) {
					string sc;
					getline(fin, sc);
					if (sc == "")break;
					int newsc = stoi(sc);
					scores.push_back(newsc);
				}
				fin.close();
				got_scores = true;
			}
		}
		else if (situation == 1) {
			font_75(U"遊び方").drawAt(600, 120, Palette::Deepskyblue);
			font_35(U"ヘビの尻尾に自分の頭をくっつけましょう！").drawAt(600, 225);
			font_35(U"頭をくっつけると、自分の長さが伸びます。").drawAt(600, 275);
			font_35(U"自分の尻尾に頭をくっつけられないように気を付けましょう!").drawAt(600, 325);
			font_35(U"左クリックしながらだと速くなります。").drawAt(600, 375);

			Rect(420, 450, 360, 60).shearedX(120).draw(Palette::Blue);
			font_35(U"戻る").drawAt(600, 480);
			if (Rect(420, 450, 360, 60).shearedX(120).mouseOver()) {
				Rect(420, 450, 360, 60).shearedX(120).drawFrame(7, Palette::Yellow);
				if (Rect(420, 450, 360, 60).shearedX(120).leftClicked()) {
					situation = 0;
				}
			}
		}
		else if (situation == 2) {
			if (gamingtime.s() >= 60) {
				situation = 3;
			}
			if (!game.isdead) {
				if (!game.update()) {
					gamingtime.reset();
					gamingtime.start();
				}
				game.draw();
				Circle(40, 40, 40).drawPie(0, ToRadians(gamingtime.s() * 6), Palette::Red);
				font_25(gamingtime.s()).drawAt(40, 40, Palette::Yellow);
				font_25(U"Score:").draw(10, 110, Palette::Lightyellow);
				font_25(score).draw(10, 135, Palette::Springgreen);
				if (Scene::FrameCount() % 10 == 0)score += game.snake.pos.size();
			}
			else {
				game.draw(0.01);
				if (gamingtime.s() >= 2)situation = 3;
			}
		}
		else {
			font_75(U"結果").drawAt(600, 120, Palette::Deepskyblue);
			font_35(U"スコア:", score).drawAt(600, 200, Palette::Lightgreen);
			if (!wrote_score) {
				ofstream fout; fout.open("GameData/Scores.txt", ios::app);
				fout << score << endl;
				fout.close(); wrote_score = true;
				scores.push_back(score);
				sort(scores.begin(), scores.end(), greater<>());
			}

			bool isused = false;
			int rank = 0;
			for (int i = 0; i < 50; i++) {
				if (i < scores.size()) {
					if (scores[i] == score && !isused) {
						isused = true;
						if (Scene::FrameCount() % 60 < 30) {
							font_25(rank + 1, U"位   ", scores[i]).draw(100 + i / 10 * 220, 250 + i % 10 * 28, HSV(36 + i * 3.5));
						}
					}
					else {
						font_25(rank + 1, U"位   ", scores[i]).draw(100 + i / 10 * 220, 250 + i % 10 * 28, HSV(36 + i * 3.5));
					}
				}
				else {
					font_25(rank + 1, U"位   -").draw(100 + i / 10 * 220, 250 + i % 10 * 28, HSV(36 + i * 3.5));
				}
				if (i < scores.size() - 1 && scores[i] != scores[i + 1]) {
					rank = i + 1;
				}
				if (i == scores.size() - 1)rank = i + 1;
			}
			Rect(420, 550, 360, 40).shearedX(120).draw(Palette::Blue);
			font_35(U"戻る").drawAt(600, 570);
			if (Rect(420, 550, 360, 40).shearedX(120).mouseOver()) {
				Rect(420, 550, 360, 40).shearedX(120).drawFrame(7, Palette::Yellow);
			}
			if (Rect(420, 550, 360, 40).shearedX(120).mouseOver()) {
				Rect(420, 550, 360, 40).shearedX(120).drawFrame(7, Palette::Yellow);
				if (Rect(420, 550, 360, 40).shearedX(120).leftClicked()) {
					situation = 0;
					got_scores = false;
					wrote_score = false;
					scores.clear();
					gamingtime.reset();
					score = 0;
				}
			}
		}
	}
}