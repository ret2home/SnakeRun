#include <bits/stdc++.h>
# include <Siv3D.hpp> // OpenSiv3D v0.4.1
using namespace std;

# define SIV3D_WINDOWS_HIGH_DPI

//座標
struct Point2D {
	double x, y;
};
//色付き座標
struct ColorPoint2D :Point2D {
	//色と透明度
	double Color, trans;
};
struct NumberEffect : IEffect
{
	Vec2 m_start;

	int m_number;

	int hsv;

	Font m_font;

	NumberEffect(const Vec2& start, int num, int color)
		: m_start(start)
		, m_number(num)
		, hsv(color)
		, m_font(FontAsset(U"font_40")) {}

	bool update(double t) override
	{
		const HSV color(180 - hsv * 1.8, 1.0 - (t * 2.0));

		m_font(m_number).drawAt(m_start.movedBy(0, t * -120), color);

		return t < 1.0;
	}
};
struct Snake {
	//ヘビのそれぞれの座標
	vector<Point2D>pos;
	double hsv;
	void update(bool automode, bool isfast, double x, double y, int difficulty) {
		//パラメーター
		double para;
		if (!automode)para = 1.0 + (pos.size() - 5) / 80.;
		else para = 1.0 + (pos.size() - 5) / 30.;

		//滑らかに動くように
		if (!automode && sqrt(pow(abs(x - pos[0].x), 2) + pow(abs(y - pos[0].y), 2)) <= 15.0)goto loop;

		//進む角度
		double degree;
		degree = 90 - ToDegrees(atan2(pos[0].y - y, x - pos[0].x));


		if (!automode && isfast && pos.size() > 5) {
			//自機、ブーストモード
			double susumu;
			if (difficulty == 0) {
				susumu = 500;
			}
			else if (difficulty == 1) {
				susumu = 530;
			}
			else susumu = 560;

			pos[0].x = pos[0].x + cos(ToRadians(90 - degree)) * Scene::DeltaTime() * susumu * para;
			pos[0].y = pos[0].y - sin(ToRadians(90 - degree)) * Scene::DeltaTime() * susumu * para;
			if (Scene::FrameCount() % 120 == 0) {
				pos.pop_back();
			}
		}
		else if (!automode) {
			//自機、通常モード
			double susumu;
			if (difficulty == 0) {
				susumu = 325;
			}
			else if (difficulty == 1) {
				susumu = 350;
			}
			else susumu = 375;
			pos[0].x = pos[0].x + cos(ToRadians(90 - degree)) * Scene::DeltaTime() * susumu * para;
			pos[0].y = pos[0].y - sin(ToRadians(90 - degree)) * Scene::DeltaTime() * susumu * para;
		}
		else if (isfast && pos.size() > 5) {
			//敵機、ブーストモード
			double susumu;
			if (difficulty == 0) {
				susumu = 300;
			}
			else if (difficulty == 1) {
				susumu = 330;
			}
			else if (difficulty == 2) {
				susumu = 360;
			}
			else {
				//隠しモード
				susumu = 480;
			}
			pos[0].x = pos[0].x + cos(ToRadians(90 - degree)) * Scene::DeltaTime() * susumu * para;
			pos[0].y = pos[0].y - sin(ToRadians(90 - degree)) * Scene::DeltaTime() * susumu * para;
			if (Scene::FrameCount() % 120 == 0) {
				pos.pop_back();
			}
		}
		else {
			//敵機、通常モード
			double susumu;
			if (difficulty == 0) {
				susumu = 220;
			}
			else if (difficulty == 1) {
				susumu = 230;
			}
			else if (difficulty == 2) {
				susumu = 240;
			}
			else {
				//隠しモード
				susumu = 330;
			}
			pos[0].x = pos[0].x + cos(ToRadians(90 - degree)) * Scene::DeltaTime() * susumu * para;
			pos[0].y = pos[0].y - sin(ToRadians(90 - degree)) * Scene::DeltaTime() * susumu * para;
		}
	loop:;

		//ヘビを滑らかに動かす
		for (int i = 1; i < pos.size(); i++) {
			degree = 90 - ToDegrees(atan2(pos[i].y - pos[i - 1].y, pos[i - 1].x - pos[i].x));
			pos[i].x = pos[i - 1].x - cos(ToRadians(90 - degree)) * 12.;
			pos[i].y = pos[i - 1].y + sin(ToRadians(90 - degree)) * 12.;
		}
	}
	void unite(int size) {
		//尾に繋げる
		double degree = Random(0, 360);
		for (int i = 0; i < size; i++) {
			double x = pos.back().x + cos(ToRadians(90 - degree)) * 12.;
			double y = pos.back().y - sin(ToRadians(90 - degree)) * 12.;
			pos.push_back({ x,y });
		}
	}
	void init(double x, double y, double size = 5) {
		hsv = Random(60, 180);
		pos.clear();
		pos.push_back({ x,y });
		unite(size - 1);
	}
};
double dis(double x, double y, double x_, double y_) {
	return sqrt(pow(abs(x - x_), 2) + pow(abs(y - y_), 2));
}
double getdegree(double x, double y, double x_, double y_) {
	return 90. - ToDegrees(atan2(y - y_, x_ - x));
}
struct Game {
	//自機
	Snake snake;
	//敵機
	vector<Snake>other;
	//中心とのずれ
	Point standard;
	//既に死んだヘビ
	vector<ColorPoint2D>dead;
	//背景
	vector<Point2D>back;
	bool isdead = false;
	//Windowのサイズ
	int windowx, windowy;
	int score = 0;
	int difficulty;
	Effect effect;
	void init(int x, int y, int dif, int size) {
		difficulty = dif;
		score = 0;
		windowx = x; windowy = y;
		isdead = false;
		other.clear();
		dead.clear();
		//自機の初期化
		snake.init((double)windowx / 2, (double)windowy / 2, 10);
		//敵機の生成
		for (int i = 0; i < size; i++) {
			Snake s;
			double x, y;
		loop:;
			x = Random(-500, 2400); y = Random(-500, 1600);
			if (x >= windowx / 2 - 500 && x <= windowx / 2 + 500 && y >= windowy / 2 - 500 && y <= windowy / 2 + 500)goto loop;
			s.init(x, y);
			other.push_back(s);
		}
		back.clear();
		//背景
		for (double i = -100; i < x + 100; i += 50) {
			for (double j = -100; j < y + 100; j += 50) {
				back.push_back({ i ,j });
			}
		}
	}
	//ゲームのアップデート処理
	bool update(int gamingtime) {
		//エフェクトのアップデート
		effect.update();

		//自機のアップデート
		snake.update(false, MouseL.pressed(), Cursor::Pos().x, Cursor::Pos().y, difficulty);

		//スコア計算
		if (Scene::FrameCount() % 60 == 0)score += snake.pos.size() * gamingtime / 10.;

		//AI主力部分、向かう方向等の計算
		for (int i = 0; i < other.size(); i++) {
			//どのヘビを追うか、どのヘビから逃げるか
			int chindex = -1, esindex = -1;
			//追う相手との最短距離
			double chase = dis(snake.pos.back().x, snake.pos.back().y, other[i].pos[0].x, other[i].pos[0].y);
			//追われる相手との最短距離
			double escape = dis(snake.pos[0].x, snake.pos[0].y, other[i].pos.back().x, other[i].pos.back().y);

			//自機から追われる可能性が高い時、回り込みをする
			if (chase >= escape && escape <= other[i].pos.size() * 12. * 3.) {
				double degree = getdegree(snake.pos[0].x, snake.pos[0].y, other[i].pos.back().x, other[i].pos.back().y);
				double x, y;
				if (gamingtime % 4 < 2) {
					double deg;
					if (difficulty == 0)deg = 30;
					else if (difficulty == 1)deg = 45;
					else deg = 60;
					x = other[i].pos[0].x + cos(ToRadians(90 - degree + deg)) * 100.;
					y = other[i].pos[0].y - sin(ToRadians(90 - degree + deg)) * 100.;
				}
				else {
					double deg;
					if (difficulty == 0)deg = 30;
					else if (difficulty == 1)deg = 45;
					else deg = 60;
					x = other[i].pos[0].x + cos(ToRadians(90 - degree - deg)) * 100.;
					y = other[i].pos[0].y - sin(ToRadians(90 - degree - deg)) * 100.;
				}
				other[i].update(true, escape < 100, x, y, difficulty);
				goto endupdate;

			}
			for (int j = 0; j < other.size(); j++) {
				if (i == j)continue;
				double di1 = dis(other[i].pos[0].x, other[i].pos[0].y, other[j].pos.back().x, other[j].pos.back().y);
				double di2 = dis(other[i].pos.back().x, other[i].pos.back().y, other[j].pos[0].x, other[j].pos[0].y);

				//他の敵機から追われる可能性が高い時、回り込む
				if (di1 >= di2 && di2 <= other[i].pos.size() * 12. * 2.) {
					double degree = getdegree(other[j].pos[0].x, other[j].pos[0].y, other[i].pos.back().x, other[i].pos.back().y);
					double x, y;
					if (gamingtime % 4 < 2) {
						x = other[i].pos[0].x + cos(ToRadians(90 - degree + 60)) * 100.;
						y = other[i].pos[0].y - sin(ToRadians(90 - degree + 60)) * 100.;
					}
					else {
						x = other[i].pos[0].x + cos(ToRadians(90 - degree - 60)) * 100.;
						y = other[i].pos[0].y - sin(ToRadians(90 - degree - 60)) * 100.;
					}
					other[i].update(true, escape < 100, x, y, difficulty);
					goto endupdate;
				}

				if (escape > di2) {
					escape = di2;
					esindex = j;
				}
				if (chase > di1&& di2 > di1) {
					chindex = j; chase = di1;
				}
			}
			if (chindex == -1) {
				//自機を追う
				other[i].update(true, chase < 100, snake.pos.back().x, snake.pos.back().y, difficulty);
			}
			else {
				//他の敵機を追う
				other[i].update(true, chase < 100, other[chindex].pos.back().x, other[chindex].pos.back().y, difficulty);
			}
		endupdate:;
		}

		//自機が敵機を食べたかの判定
		Circle tip(snake.pos[0].x, snake.pos[0].y, (difficulty == 0 ? 40 : 30));
		for (int i = 0; i < other.size(); i++) {
			if (tip.intersects(Circle(other[i].pos.back().x, other[i].pos.back().y, 10))) {
				score += other[i].pos.size() * 100;
				snake.unite(1);
				effect.add<NumberEffect>(Vec2(snake.pos[0].x, snake.pos[0].y), other[i].pos.size() * 100, Random(0, 100));
				AudioAsset(U"eat").stop(); AudioAsset(U"eat").play();
				for (int j = 0; j < other[i].pos.size(); j++) {
					dead.push_back({ other[i].pos[j].x,other[i].pos[j].y,other[i].hsv + 60. / other[i].pos.size() * j,1. });
				}
				other[i] = other.back(); other.pop_back();
				Snake s; double x, y;
			loop:;
				x = Random(-500, 2400); y = Random(-500, 1600);
				if (x >= 0 && x <= windowx && y >= 0 && y <= windowy)goto loop;
				s.init(x, y, 5 + gamingtime / 6);
				other.push_back(s);
				return true;
			}
			if (other[i].pos[0].x < -500 || other[i].pos[0].x > 2400 || other[i].pos[0].y < -500 || other[i].pos[0].y > 1600) {
				Snake s; double x, y;
			loop2:;
				x = Random(-500, 2400); y = Random(-500, 1600);
				if (x >= 0 && x <= windowx && y >= 0 && y <= windowy)goto loop2;
				s.init(x, y, 5 + gamingtime / 6);
				other[i] = s;
			}
		}
		//敵機が他の敵機を食べたかの判定
		for (int i = 0; i < other.size(); i++) {
			Circle c1(other[i].pos[0].x, other[i].pos[0].y, 10);
			for (int j = 0; j < other.size(); j++) {
				if (i == j)continue;
				if (c1.intersects(Circle(other[j].pos.back().x, other[j].pos.back().y, 10))) {
					other[i].unite(1);
					for (int k = 0; k < other[j].pos.size(); k++) {
						dead.push_back({ other[j].pos[k].x,other[j].pos[k].y,other[j].hsv + 60. / other[j].pos.size() * k,1.0 });
					}
					other[j] = other.back(); other.pop_back();
					Snake s;
					double x, y;
				loop3:;
					x = Random(-500, 2400); y = Random(-500, 1600);
					if (x >= 0 && x <= windowx && y >= 0 && y <= windowy)goto loop3;
					s.init(x, y, 5 + gamingtime / 6);
					other.push_back(s);
					return true;
				}
			}
		}
		//既に透明度が0以下になっているかチェック
		for (int i = 0; i < dead.size(); i++) {
			if (dead[i].trans <= 0.0) {
				dead[i] = dead.back(); dead.pop_back();
			}
		}
		//自機が敵機に食べられたかのチェック
		Circle end(snake.pos.back().x, snake.pos.back().y, 10);
		for (int i = 0; i < other.size(); i++) {
			if (end.intersects(Circle(other[i].pos[0].x, other[i].pos[0].y, 10))) {
				for (int j = 0; j < snake.pos.size(); j++) {
					dead.push_back({ snake.pos[j].x,snake.pos[j].y,60 + 180. / snake.pos.size() * j,1.0 });
				}
				isdead = true;
				return false;
			}
		}
		return true;
	}
	//描画
	void draw(double speed = 0.03) {
		//画面の中心と自機とのずれを計算
		standard.x = windowx / 2 - snake.pos[0].x;
		standard.y = windowy / 2 - snake.pos[0].y;

		//背景のずれを計算、修正

		int diffx = (windowx + 100 - back.back().x) / 50, diffy = (windowy + 100 - back.back().y) / 50;
		for (int i = 0; i < back.size(); i++) {
			if (!isdead) {
				back[i].x += standard.x;
				back[i].y += standard.y;
				back[i].x += diffx * 50;
				back[i].y += diffy * 50;
			}
			if (back[i].x >= -100 && back[i].x <= windowx + 100 && back[i].y >= -100 && back[i].y <= windowy + 100) {
				Rect(back[i].x, back[i].y, 40, 40).draw(Palette::Black);
			}
		}

		//敵機の描画
		for (int i = 0; i < other.size(); i++) {
			for (int j = other[i].pos.size() - 1; j >= 0; j--) {
				if (standard.x + other[i].pos[j].x >= 0 && standard.x + other[i].pos[j].x <= windowx && standard.y + other[i].pos[j].y >= 0 && standard.y + other[i].pos[j].y <= windowy) {
					Circle(standard.x + other[i].pos[j].x, standard.y + other[i].pos[j].y, 12).draw(HSV(other[i].hsv + 60. / other[i].pos.size() * j)).drawFrame(0, 1, Palette::White);
				}
				if (!isdead) {
					//自機との相対距離を修正
					other[i].pos[j].x += standard.x; other[i].pos[j].y += standard.y;
				}
			}
			//ヘビの目を描画
			double degree = getdegree(other[i].pos[1].x, other[i].pos[1].y, other[i].pos[0].x, other[i].pos[0].y);
			if (isdead) {
				Circle(other[i].pos[0].x + cos(ToRadians(90 - degree + 45)) * 7 + standard.x, other[i].pos[0].y - sin(ToRadians(90 - degree + 45)) * 7 + standard.y, 5).draw(Palette::White);
				Circle(other[i].pos[0].x + cos(ToRadians(90 - degree - 45)) * 7 + standard.x, other[i].pos[0].y - sin(ToRadians(90 - degree - 45)) * 7 + standard.y, 5).draw(Palette::White);
				Circle(other[i].pos[0].x + cos(ToRadians(90 - degree + 45)) * 7 + standard.x, other[i].pos[0].y - sin(ToRadians(90 - degree + 45)) * 7 + standard.y, 3).draw(Palette::Black);
				Circle(other[i].pos[0].x + cos(ToRadians(90 - degree - 45)) * 7 + standard.x, other[i].pos[0].y - sin(ToRadians(90 - degree - 45)) * 7 + standard.y, 3).draw(Palette::Black);
			}
			else {
				Circle(other[i].pos[0].x + cos(ToRadians(90 - degree + 45)) * 7, other[i].pos[0].y - sin(ToRadians(90 - degree + 45)) * 7, 5).draw(Palette::White);
				Circle(other[i].pos[0].x + cos(ToRadians(90 - degree - 45)) * 7, other[i].pos[0].y - sin(ToRadians(90 - degree - 45)) * 7, 5).draw(Palette::White);
				Circle(other[i].pos[0].x + cos(ToRadians(90 - degree + 45)) * 7, other[i].pos[0].y - sin(ToRadians(90 - degree + 45)) * 7, 3).draw(Palette::Black);
				Circle(other[i].pos[0].x + cos(ToRadians(90 - degree - 45)) * 7, other[i].pos[0].y - sin(ToRadians(90 - degree - 45)) * 7, 3).draw(Palette::Black);
			}
		}
		//自機を描画
		if (!isdead) {
			Circle(snake.pos[0].x, snake.pos[0].y, (difficulty == 0 ? 40 : 30)).draw(HSV(0, 0.3));
			for (int i = snake.pos.size() - 1; i >= 0; i--) {
				if (standard.x + snake.pos[i].x >= 0 && standard.x + snake.pos[i].x <= windowx && standard.y + snake.pos[i].y >= 0 && standard.y + snake.pos[i].y <= windowy) {
					Circle(standard.x + snake.pos[i].x, standard.y + snake.pos[i].y, 12).draw(HSV(60 + 180. / snake.pos.size() * i)).drawFrame(0, 1, Palette::White);
				}
				//中心とのずれを修正
				snake.pos[i].x += standard.x;
				snake.pos[i].y += standard.y;
			}
			//目を描画
			double degree = getdegree(snake.pos[1].x, snake.pos[1].y, snake.pos[0].x, snake.pos[0].y);
			Circle(snake.pos[0].x + cos(ToRadians(90 - degree + 45)) * 7, snake.pos[0].y - sin(ToRadians(90 - degree + 45)) * 7, 5).draw(Palette::White);
			Circle(snake.pos[0].x + cos(ToRadians(90 - degree - 45)) * 7, snake.pos[0].y - sin(ToRadians(90 - degree - 45)) * 7, 5).draw(Palette::White);
			Circle(snake.pos[0].x + cos(ToRadians(90 - degree + 45)) * 7, snake.pos[0].y - sin(ToRadians(90 - degree + 45)) * 7, 3).draw(Palette::Black);
			Circle(snake.pos[0].x + cos(ToRadians(90 - degree - 45)) * 7, snake.pos[0].y - sin(ToRadians(90 - degree - 45)) * 7, 3).draw(Palette::Black);
		}
		//死んだヘビのアニメーション
		for (int i = 0; i < dead.size(); i++) {
			if (standard.x + dead[i].x >= 0 && standard.x + dead[i].x <= windowx && standard.y + dead[i].y >= 0 && standard.y + dead[i].y <= windowy) {
				Circle(standard.x + dead[i].x, standard.y + dead[i].y, 12).draw(HSV(dead[i].Color, dead[i].trans));
			}
			if (!isdead) {
				dead[i].x += standard.x;
				dead[i].y += standard.y;
			}
			dead[i].trans -= speed;
		}
	}
};
//ボタン
bool button(String arg1, Vec2 arg2, Vec2 arg3, HSV arg4) {
	RoundRect rec(arg2.x, arg2.y, arg3.x, arg3.y, 10);
	if (!rec.mouseOver()) {
		rec.drawShadow(Vec2(8, 8), 30, 5).draw(arg4);
	}
	else {
		rec.draw(arg4);
	}
	FontAsset(U"font_40")(arg1).drawAt(arg2.x + arg3.x / 2, arg2.y + arg3.y / 2, Palette::Black);
	return rec.leftClicked();
}
void Main() {
	Window::Resize(1200, 600);
	Window::SetTitle(U"Snake Run");
	//ゲーム本体
	Game game;
	//シチュエーション
	int situation = 0;
	//スコアを読み込んだか
	bool got_scores = false;
	//記録含めたスコア
	vector<int>scores;

	//フォント読み込み
	FontAsset::Register(U"font_40", 40);
	FontAsset::Register(U"font_30", 30, Typeface::Bold);
	FontAsset::Register(U"title", 120, Resource(U"GameData/title.otf"));
	FontAsset::Register(U"ranking", 40, Resource(U"GameData/title.otf"));

	//ストップウォッチ
	Stopwatch gamingtime;

	//音楽データ
	AudioAsset::Register(U"eat", Resource(U"GameData/eat.mp3"), AssetParameter::LoadAsync());
	AudioAsset::Register(U"die", Resource(U"GameData/die.mp3"), AssetParameter::LoadAsync());
	Audio gameplay(Resource(U"GameData/gameplay.mp3"), Arg::loop = true);

	//解像度取得
	const Array<Size> resolutions = Graphics::GetFullscreenResolutions();

	//画像読み込み
	Texture background(Resource(U"GameData/background.jpg"), TextureDesc::Mipped);
	Texture howtoeat(Resource(U"GameData/eat.png"), TextureDesc::Mipped);
	Texture first(Resource(U"GameData/first.png"), TextureDesc::Mipped);
	Texture second(Resource(U"GameData/second.png"), TextureDesc::Mipped);
	Texture third(Resource(U"GameData/third.png"), TextureDesc::Mipped);

	//隠しコマンド処理
	String password, input = U"";
	TextReader reader(Resource(U"GameData/password.txt"));
	reader.readLine(password);
	reader.close();

	while (System::Update()) {
		if (situation == 0) {
			//スタート画面
			background(0, 400, 1600, 1200).resized(1200, 600).draw(ColorF(0.7));
			String title = U"SnakeRun";
			for (int i = 0; i < title.size(); i++) {
				FontAsset(U"title")(title[i]).draw(200 + i * 100, 80, HSV(i * 45));
			}

			if (button(U"あそぶ", Vec2(300, 270), Vec2(600, 60), HSV(30))) {
				situation = 2;
			}
			if (button(U"あそびかた", Vec2(400, 370), Vec2(400, 60), HSV(120))) {
				situation = 1;
			}
		}
		else if (situation == 1) {
			//遊び方の画面
			background(0, 400, 1600, 1200).resized(1200, 600).draw(ColorF(0.7));

			howtoeat.drawAt(200, 325);
			FontAsset(U"title")(U"あそびかた").drawAt(600, 120, HSV(120));
			FontAsset(U"font_30")(U"他のヘビの尾に頭をくっつけて食べましょう！").drawAt(750, 225, Palette::Yellow);
			FontAsset(U"font_30")(U"頭をくっつけると、自分の長さが伸びます。").drawAt(750, 265, Palette::Black);
			FontAsset(U"font_30")(U"ヘビは、カーソルの方向に進みます。").drawAt(750, 305, Palette::Yellow);
			FontAsset(U"font_30")(U"左クリックをしている間は加速しますが、長さが縮みます。").drawAt(750, 345, Palette::Black);
			FontAsset(U"font_30")(U"縮みすぎると、加速できなくなります。").drawAt(750, 385, Palette::Black);
			FontAsset(U"font_30")(U"くれぐれも自分が食べられないように！").drawAt(750, 425, Palette::Yellow);

			if (button(U"もどる", Vec2(400, 500), Vec2(400, 60), HSV(210))) {
				situation = 0;
			}
		}
		else if (situation == 2) {
			//レベル選択画面
			background(0, 400, 1600, 1200).resized(1200, 600).draw(ColorF(0.7));
			FontAsset(U"title")(U"レベルをえらぶ").drawAt(600, 120, HSV(60));
			if (button(U"かんたん", Vec2(300, 230), Vec2(600, 60), HSV(120))) {
				situation = 3;
				game.init(resolutions.back().x, resolutions.back().y, 0, 15); gamingtime.start();
				gameplay.play();
				Window::SetFullscreen(true, resolutions.back()); input = U"";
			}
			else if (button(U"ふつう", Vec2(300, 330), Vec2(600, 60), HSV(60))) {
				situation = 3;
				game.init(resolutions.back().x, resolutions.back().y, 1, 20); gamingtime.start();
				gameplay.play();
				Window::SetFullscreen(true, resolutions.back()); input = U"";
			}
			else if (button(U"むずかしい", Vec2(300, 430), Vec2(600, 60), HSV(30))) {
				situation = 3;
				game.init(resolutions.back().x, resolutions.back().y, 2, 25); gamingtime.start();
				gameplay.play();
				Window::SetFullscreen(true, resolutions.back()); input = U"";
			}

			//隠しコマンド
			TextInput::UpdateText(input);
			if (input == password) {
				situation = 3;
				game.init(resolutions.back().x, resolutions.back().y, 3, 30); gamingtime.start();
				gameplay.play();
				Window::SetFullscreen(true, resolutions.back()); input = U"";
			}

			if (button(U"もどる", Vec2(400, 520), Vec2(400, 60), HSV(210))) {
				situation = 0;
				input = U"";
			}
		}
		else if (situation == 3) {
			//ゲーム画面

			//時間切れ
			if (gamingtime.s() >= 60) {
				situation = 4;
				Window::SetFullscreen(false);
				gameplay.stop();
			}

			if (!game.isdead) {
				//ゲームの描画
				game.draw();
				//自機が食べられた場合
				if (!game.update(gamingtime.s())) {
					gamingtime.reset();
					gamingtime.start();
					gameplay.stop();
					AudioAsset(U"die").play();
				}
				//残り時間
				Circle(80, 80, 80).drawPie(0, ToRadians(gamingtime.s() * 6), Palette::Red);
				FontAsset(U"ranking")(60 - gamingtime.s()).drawAt(80, 80, Palette::Yellow);
				//スコア等表示
				FontAsset(U"ranking")(U"スコア").draw(10, 200, Palette::Lightyellow);
				FontAsset(U"ranking")(game.score).draw(10, 260, Palette::Springgreen);
				FontAsset(U"ranking")(U"レベル").draw(10, 350, Palette::Lightyellow);
				String level;
				if (game.difficulty == 0) {
					level = U"かんたん";
				}
				else if (game.difficulty == 1) {
					level = U"ふつう";
				}
				else if (game.difficulty == 2) {
					level = U"むずかしい";
				}
				else {
					level = U"さいきょう";
				}
				FontAsset(U"ranking")(level).draw(10, 410, Palette::Springgreen);
				//一定の時間になったら残り時間を表示
				if (gamingtime.s() == 30 || gamingtime.s() == 45 || gamingtime.s() == 50 ||  gamingtime.s() >= 55 && gamingtime.ms() % 1000 <= 200) {
					Circle(resolutions.back().x / 2, resolutions.back().y / 2, 160).drawPie(0, ToRadians(gamingtime.s() * 6), HSV(0, 0.3));
					FontAsset(U"title")(60 - gamingtime.s()).drawAt(resolutions.back().x / 2, resolutions.back().y / 2, HSV(60, 0.3));
				}
			}
			else {
				//既に死んでいる場合のアニメーション
				game.effect.update();
				game.draw(0.01);
				if (gamingtime.s() >= 2) {
					situation = 4;
					Window::SetFullscreen(false);
				}
			}
		}
		else {
			//結果表示
			background(0, 400, 1600, 1200).resized(1200, 600).draw(ColorF(0.7));
			FontAsset(U"title")(U"ランキング").drawAt(600, 120, HSV(60));
			String level;
			if (game.difficulty == 0) {
				level = U"かんたん";
			}
			else if (game.difficulty == 1) {
				level = U"ふつう";
			}
			else if (game.difficulty == 2) {
				level = U"むずかしい";
			}
			else {
				level = U"さいきょう";
			}
			FontAsset(U"font_30")(U"レベル:", level, U"　　スコア:", game.score).drawAt(600, 225, Palette::Midnightblue);
			//データ読み込み
			if (!got_scores) {
				ifstream fin; fin.open("GameData/Scores.txt", ios::in);
				while (!fin.eof()) {
					string sc;
					getline(fin, sc);
					if (sc == "")break;
					int newsc = stoi(sc);
					if (newsc / 100000000 == game.difficulty)scores.push_back(newsc % 100000000);
				}
				fin.close();
				got_scores = true;
				ofstream fout; fout.open("GameData/Scores.txt", ios::app);
				fout << game.difficulty * 100000000 + game.score << endl;
				fout.close();
				scores.push_back(game.score);
				sort(scores.begin(), scores.end(), greater<>());
			}

			//ランキング表示


			bool isused = false;

			if (scores.size()) {
				if (scores[0] != game.score || Scene::FrameCount() % 60 <= 30) {
					FontAsset(U"ranking")(scores[0]).draw(550, 265, Palette::Gold);
					first.resized(72, 50).draw(460, 260);
				}
				if (scores[0] == game.score)isused = true;
			}
			else {
				FontAsset(U"ranking")(U"-").draw(550, 265, Palette::Gold);
				first.resized(72, 50).draw(460, 260);
			}
			if (scores.size() >= 2) {
				if (scores[1] != game.score || Scene::FrameCount() % 60 <= 30 || isused) {
					FontAsset(U"ranking")(scores[1]).draw(550, 315, Palette::Silver);
					second.resized(72, 50).draw(460, 310);
				}
				if (scores[1] == game.score)isused = true;
			}
			else {
				FontAsset(U"ranking")(U"-").draw(550, 315, Palette::Silver);
				second.resized(72, 50).draw(460, 310);
			}
			if (scores.size() >= 3) {
				if (scores[2] != game.score || Scene::FrameCount() % 60 <= 30 || isused) {
					FontAsset(U"ranking")(scores[2]).draw(550, 365, Palette::Darkgoldenrod);
					third.resized(72, 50).draw(460, 360);
				}
				if (scores[2] == game.score)isused = true;
			}
			else {
				FontAsset(U"ranking")(U"-").draw(550, 365, Palette::Darkgoldenrod);
				third.resized(72, 50).draw(460, 360);
			}

			int rank = 3;
			for (int i = 3; i < 5; i++) {
				if (i < scores.size()) {
					if (scores[i] == game.score && !isused) {
						isused = true;
						if (Scene::FrameCount() % 60 < 30) {
							FontAsset(U"ranking")(rank + 1, U"  ", scores[i]).draw(480, 265 + i * 50, Palette::Cyan);
						}
					}
					else {
						FontAsset(U"ranking")(rank + 1, U"  ", scores[i]).draw(480, 265 + i * 50, Palette::Cyan);
					}
				}
				else {
					FontAsset(U"ranking")(rank + 1, U"  -").draw(480, 265 + i * 50, Palette::Cyan);
				}
				if (i < scores.size() - 1 && scores[i] != scores[i + 1]) {
					rank = i + 1;
				}
				if (i == scores.size() - 1)rank = i + 1;
			}

			//隠しコマンド表示
			if (game.difficulty == 2 && game.score >= 10000) {
				FontAsset(U"font_30")(U"レベル選択画面で\n", password, U"と入力すると\nさいきょうモードが遊べます！").drawAt(950, 365,Palette::Black);
			}

			if (button(U"もどる", Vec2(400, 520), Vec2(400, 60), HSV(210))) {
				situation = 0;
				got_scores = false;
				scores.clear();
				gamingtime.reset();
			}
		}
	}
}