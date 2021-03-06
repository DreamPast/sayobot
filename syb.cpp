#ifndef WIN32
    #define sprintfS(buffer, length, format, ...) sprintf(buffer, format, __VA_ARGS__)
#else
	#define sprintfS sprintf_s
#endif

#include <math.h>
#include <sys/stat.h>
#include <time.h>

#include <fstream>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#ifndef WIN32
#define MAGICKCORE_QUANTUM_DEPTH 16
#ifndef MAGICKCORE_HDRI_ENABLE
#define MAGICKCORE_HDRI_ENABLE TRUE
#endif
#endif
#include <Magick++.h>
#include <random>
#include <string>

namespace Sayobot
{
    struct TextStyle {
        TextStyle(
            const std::string& _color = "black", const double _pointsize = 12.0f,
            const std::string& _font_family = "monospace",
            const MagickCore::GravityType _gravity =
                MagickCore::GravityType::UndefinedGravity,
            const MagickCore::AlignType _align = MagickCore::AlignType::UndefinedAlign)
        {
            color = _color;
            pointsize = _pointsize;
            font_family = _font_family;
            gravity = _gravity;
            align = _align;
        }
        std::string color;
        double pointsize;
        std::string font_family;
        MagickCore::GravityType gravity;
        MagickCore::AlignType align;
    };

    class Image {
    public:
        Image()
        {
        }

        void Create(const size_t &width, const size_t &height)
        {
            this->image.size(Magick::Geometry(width, height));
            //this->image.read("xc:#FFFFFF");
        }

        void ReadFromFile(const std::string& path)
        {
            this->image.read(path);
        }

        void ReadFromUrl(const std::string& url)
        {
            this->image = Magick::Image(url);
        }

        void Crop(const Magick::Geometry& geometry)
        {
            this->image.crop(geometry);
        }

        void Crop(const size_t width, const size_t height, const size_t x_offset,
                  const size_t y_offset)
        {
            this->image.crop(Magick::Geometry(width, height, x_offset, y_offset));
        }

        void Rotate(const double degrees)
        {
            this->image.rotate(degrees);
        }

        void Drawtext(const std::string& str, const TextStyle& textStyle,
                      double x_offset, double y_offset)
        {
            Magick::DrawableList drawableList;
            drawableList.push_back(Magick::DrawableFillColor(textStyle.color));
            std::string ext =
                textStyle.font_family.substr(textStyle.font_family.find_last_of('.'));
            drawableList.push_back(Magick::DrawableFont(textStyle.font_family));
            drawableList.push_back(Magick::DrawablePointSize(textStyle.pointsize));
            drawableList.push_back(Magick::DrawableText(x_offset, y_offset, str));
            drawableList.push_back(Magick::DrawableGravity(textStyle.gravity));
            drawableList.push_back(Magick::DrawableTextAlignment(textStyle.align));
            this->image.draw(drawableList);
        }
        /*
         在图上绘制文字
         * 参数列表:
         *** str (const std::string&) 绘制的文字
         *** x_offset (double) 相对于起始点 (0, 0) 的x坐标偏移量
         *** y_offset (double) 相对于起始点 (0, 0) 的y坐标偏移量
         *** Color (const std::string&) 绘制的颜色 ( red 或者 #FF0000 都是可行的)
         *** size (double) 字体大小
         *** fontFamily (const std::string&)
             字体名或字体文件路径（如果是TTF格式的字体文件，最前面需要加@）
         *** 可选 opacity (double) 不透明度
         *** 可选 gravity 字体重力对齐方式

         ***** 以下TTF字体不可使用
         *** 可选 align 字体左右对齐方式
         *** 可选 stretch 字体扩张方式
         *** 可选 weight 字体粗细值，400为标准
        */
        void Drawtext(
            const std::string& str, double x_offset, double y_offset,
            const std::string& Color, double size, const std::string& fontFamily,
            const MagickCore::GravityType gravity =
                MagickCore::GravityType::UndefinedGravity,
            const MagickCore::AlignType align = MagickCore::AlignType::UndefinedAlign)
        {
            Magick::DrawableList drawableList;
            drawableList.push_back(Magick::DrawableFillColor(Color));
            drawableList.push_back(Magick::DrawableTextAlignment(align));
            drawableList.push_back(Magick::DrawableFont(fontFamily));
            drawableList.push_back(Magick::DrawablePointSize(size));
            drawableList.push_back(Magick::DrawableText(x_offset, y_offset, str));
            drawableList.push_back(Magick::DrawableGravity(gravity));
            this->image.draw(drawableList);
        }

        /*
         * 在图上贴画上另一张图
         * 参数列表:
         *** image (const Image&) 另一张图
         *** x_offset (size_t) 相对于起始点 (0, 0) 的x坐标偏移量
         *** y_offset (size_t) 相对于起始点 (0, 0) 的y坐标偏移量
         *** 可选 width 重新调整图片宽度
         *** 可选 height 重新调整图片高度
         */
        void DrawPic(Image& image, const size_t x_offset, const size_t y_offset,
                     size_t width = 0, size_t height = 0)
        {
            if (width && height)
                image.resize(Magick::Geometry(width, height));
            this->image.composite(
                image.image, x_offset, y_offset, MagickCore::OverCompositeOp);
        }

        /*
         * 在图上贴画上另一张图
         * 参数列表:
         *** path (const std::string&) 另一张图的路径
         *** x_offset (size_t) 相对于起始点 (0, 0) 的x坐标偏移量
         *** y_offset (size_t) 相对于起始点 (0, 0) 的y坐标偏移量
         *** 可选 width 重新调整图片宽度
         *** 可选 height 重新调整图片高度
         */
        void DrawPic(const std::string& path, size_t x_offset, size_t y_offset,
                     size_t width = 0, size_t height = 0)
        {
            Magick::Image newImage;
            newImage.read(path);

            if (width && height)
                newImage.resize(Magick::Geometry(width, height));
            this->image.composite(
                newImage, x_offset, y_offset, MagickCore::OverCompositeOp);
        }

        std::string GetRandomHash(int length = 16)
        {
            std::default_random_engine random(time(NULL));
            std::uniform_int_distribution<int> dist(0, 128);
            int randint = dist(random);
            return std::string(this->image.perceptualHash()).substr(randint, length);
        }

        std::string GetFullHash()
        {
            return std::string(this->image.perceptualHash());
        }
        /*
         * 保存图片
         */
        void Save(const std::string& path)
        {
            this->image.quality(100);
            this->image.write(path);
        }

        void resize(const Magick::Geometry& geometry)
        {
            this->image.resize(geometry);
        }

        void resize(size_t width, size_t height)
        {
            this->image.resize(Magick::Geometry(width, height));
        }

        // 汉明距离：
        // 比较两张图片的感知哈希值，计算出不同位的数量
        // 9% 以下 认为是相似
        static double HanmingDistance(const std::string& pHash1,
                                      const std::string& pHash2)
        {
            if (pHash1.size() != pHash2.size())
                return -1;
            int difference = 0;
            for (int i = 0; i < pHash1.size(); ++i)
            {
                if (pHash1[i] != pHash2[i])
                    ++difference;
            }
            return difference * 100.0 / pHash1.size();
        }

    private:
        Magick::Image image;
    };
} // namespace Sayobot


#define BIG_POINTSIZE 54.0
#define MID_POINTSIZE 43.0
#define SMALL_POINTSIZE 29.0

#ifndef SAYOBOT_API
    #if defined __WINDOWS__ || defined _WIN32 || defined _WIN64
        #define SAYOBOT_API __declspec(dllexport) // 在Windows上，必须明示导出函数
    #else
        #define SAYOBOT_API
    #endif
#endif
#define Sayobot_Free(ptr) if(ptr) delete ptr

typedef std::basic_string<char> string_t;

string_t llToString(int64_t n) {
    const char digit[4] = {' ', 'K', 'M', 'G'};
    long double v = n;
    int a = 0;
    while (v > 1000.0) {
        v /= 1000.0;
        ++a;
    }
    char *buf = new char[20];
    sprintfS(buf, 20, "%.2Lf", v);
    string_t ret(buf);
    delete[] buf;
    return ret + digit[a];
}

string_t DoubleRound2(double a) {
    char *buf = new char[20];
    sprintfS(buf, 20, "%.2lf", a);
    string_t ret(buf);
    delete[] buf;
    return ret;
}

std::string syb_background = "../png/stat/";
std::string syb_edge = "../png/tk/";
std::string syb_font = "../fonts/";
std::string syb_skin = "../png/rank/";
std::string syb_country = "../png/country/";
std::string syb_global = "../png/world/s.png";
std::string syb_avatar = "../png/avatars/";

struct {
    std::string profile = "10014.ttf";
    std::string data = "10014.ttf";
    std::string sign = "10014.ttf";
    std::string time = "10014.ttf";
    std::string arrow = "10014.ttf";
    std::string name = "10014.ttf";
} syb_font_set;


extern "C" {

#define SAYOBOT_SET(k, lv) if(!strcmp(key, k)) return (value ? ((lv) = value).c_str() : (lv).c_str());

// 导出函数：设置路径
SAYOBOT_API const char* Sayobot_SetPath(const char* key, const char* value) {
	SAYOBOT_SET("background", syb_background)
    else SAYOBOT_SET("edge", syb_edge)
    else SAYOBOT_SET("font", syb_font)
    else SAYOBOT_SET("skin", syb_skin)
    else SAYOBOT_SET("country", syb_country)
    else SAYOBOT_SET("global", syb_global)
    else SAYOBOT_SET("avatar", syb_avatar)
    else return NULL;
}

// 导出函数：设置字体
SAYOBOT_API const char* Sayobot_SetFont(const char* key, const char* value) {
    SAYOBOT_SET("profile", syb_font_set.profile)
    else SAYOBOT_SET("data", syb_font_set.data)
    else SAYOBOT_SET("sign", syb_font_set.sign)
    else SAYOBOT_SET("time", syb_font_set.time)
    else SAYOBOT_SET("arrow", syb_font_set.arrow)
    else SAYOBOT_SET("name", syb_font_set.name)
    else return NULL;
}
#undef SAYOBOT_SET

// 导出函数：以路径初始化（仅在Windows上或者部分Mac OS上需要）
SAYOBOT_API void Sayobot_LoadMagic(const char* path) {
    Magick::InitializeMagick(path);
}

SAYOBOT_API struct UserConfigData {
    int32_t user_id;
    int64_t qq;
    char* username = nullptr;
    char* sign = nullptr;
    char* background = nullptr;
    struct {
        char* profile = nullptr;
        char* data = nullptr;
        char* sign = nullptr;
    } edge;
    /*
    struct {
        char* profile = nullptr;
        char* data = nullptr;
        char* sign = nullptr;
        char* time = nullptr;
        char* arrow = nullptr;
        char* name = nullptr;
    } font;*/
    struct {
        char* profile = nullptr;
        char* data = nullptr;
        char* sign = nullptr;
        char* time = nullptr;
        char* arrowup = nullptr;
        char* arrowdown = nullptr;
        char* name = nullptr;
    } color;
    char* skin = nullptr;
    int opacity;
};

SAYOBOT_API void Sayobot_destroy(UserConfigData* src) {
    Sayobot_Free(src->username);
    Sayobot_Free(src->sign);
    Sayobot_Free(src->background);
    Sayobot_Free(src->skin);

    Sayobot_Free(src->edge.profile);
    Sayobot_Free(src->edge.data);
    Sayobot_Free(src->edge.sign);
    /*
    Sayobot_Free(src->font.profile);
    Sayobot_Free(src->font.data);
    Sayobot_Free(src->font.sign);
    Sayobot_Free(src->font.time);
    Sayobot_Free(src->font.arrow);
    Sayobot_Free(src->font.name);
    */

    Sayobot_Free(src->color.profile);
    Sayobot_Free(src->color.data);
    Sayobot_Free(src->color.sign);
    Sayobot_Free(src->color.time);
    Sayobot_Free(src->color.arrowup);
    Sayobot_Free(src->color.arrowdown);
    Sayobot_Free(src->color.name);
}

enum mode_enum { _std = 0, taiko, ctb, mania };
SAYOBOT_API struct UserStatData {
    int32_t user_id;
    int64_t qq;
    char* username = nullptr;
    int64_t total_score, ranked_score;
    int32_t total_hit;
    double accuracy;
    float pp, level;
    int32_t global_rank, country_rank;
    char* country = nullptr;
    int64_t playcount;
    int32_t xh, x, sh, s, a;
    time_t update_timestamp;
    mode_enum mode;
    unsigned days;
};

SAYOBOT_API void UserStatData_destroy(UserStatData* src) {
    Sayobot_Free(src->username);
    Sayobot_Free(src->country);
}

SAYOBOT_API struct user_info {
    int user_id;
    char* username = nullptr;
    long registed_timestamp;
    int n300, n100, n50;
    int playcount;
    int64_t total_score, ranked_score;
    int64_t total_hits;
    float pp;
    int country_rank, global_rank;
    int count_ssh, count_ss, count_sh, count_s, count_a;
    int playtime;
    float level;
    double accuracy;
    char* country = nullptr;

};

SAYOBOT_API void user_info_destory(user_info* src) {
    Sayobot_Free(src->username);
    Sayobot_Free(src->country);
}

SAYOBOT_API struct UserPanelData {
    mode_enum mode;
    user_info uinfo;
    UserConfigData config;
    UserStatData stat;
    int compareDays;
};

// 导出函数：制作卡片，输出路径必须要带有后缀名和路径（之前默认输出到data/image下）
SAYOBOT_API const char* MakePersonalCard(const UserPanelData* data, const char* out_path) {
    const std::vector<string_t> mode_str = {"/mode-osu-med.png",
                                                "/mode-taiko-med.png",
                                                "/mode-fruits-med.png",
                                                "/mode-mania-med.png"};
    const std::vector<string_t> rank_str = {"/ranking-X-small.png",
                                                "/ranking-XH-small.png",
                                                "/ranking-S-small.png",
                                                "/ranking-SH-small.png",
                                                "/ranking-A-small.png"};               
            
        char stemp[512];
        int64_t itemp;
        float ftemp;
        double dtemp;
        Sayobot::Image image;
        image.Create(1080, 1920);
#pragma region drawing
        // 绘制背景
        sprintf(stemp, "%s%s", syb_background.c_str(), data->config.background);
        image.DrawPic(stemp, 0, 0);
        // 不透明贴图
        sprintf(stemp, "../png/fx%d.png", data->config.opacity);
        image.DrawPic(stemp, 0, 0);
        // 绘制个人信息框
        sprintf(stemp, "%s%s", syb_edge.c_str(), data->config.edge.profile);
        image.DrawPic(stemp, 50, 20, 970, 600);
        // 绘制数据框
        for (int i = 0; i < 6; ++i) {
            sprintf(stemp, "%s%s", syb_edge.c_str(), data->config.edge.data);
            image.DrawPic(stemp, 56 + 33.5 * i, 980 + 140 * i, 820, 140);
        }

        // 绘制签名框
        sprintf(stemp, "%s%s", syb_edge.c_str(), data->config.edge.sign);
        image.DrawPic(stemp, 125, 570, 825, 150);
        // 绘制头像
        sprintfS(stemp, 512, "%s%d.png", syb_avatar.c_str(), data->uinfo.user_id);
        try {
            image.DrawPic(stemp, 165, 150, 350, 350);
        } catch (Magick::Exception &ex) {
            image.DrawPic(syb_avatar + "no-avatar.png", 165, 150, 350, 350);
        }
        // 绘制模式图标
        sprintf(stemp,
                "%s%s%s",
                syb_skin.c_str(),
                data->config.skin,
                mode_str[(int)data->mode].c_str());
        image.DrawPic(stemp, 165, 150, 80, 80);
        // 绘制地球图标
        image.DrawPic(syb_global, 510, 150, 100, 100);
        // 绘制国旗
        sprintf(stemp,
                "%s%s.png", syb_country.c_str(),
                (data->uinfo.country && *data->uinfo.country) ? data->uinfo.country : "__");
        image.DrawPic(stemp, 560, 425, 80, 80);

        // 绘制rank图标
        for (int i = 0; i < 5; ++i) {
            sprintf(stemp,
                    "%s%s%s", syb_skin.c_str(),
                    data->config.skin,
                    rank_str[i].c_str());
            image.DrawPic(stemp, 165 + 120 * i, i % 2 ? 870 : 720, 82, 98);
        }
        Sayobot::TextStyle ts;
        // 绘制天数
        ts.color = data->config.color.time;
        ts.font_family = syb_font; ts.font_family += syb_font_set.time;
        ts.pointsize = SMALL_POINTSIZE;
        if (data->compareDays != 0) {
            sprintfS(stemp, 512, "compare with %u days ago", data->compareDays);
            image.Drawtext(stemp, ts, 30, 1840);
        }
        // 绘制时间
        time_t tt;
        time(&tt);
        strftime(
            stemp, 512, "%F %a %T by Sayobot with C++ & Magick++", localtime(&tt));
        image.Drawtext(stemp, ts, 30, 1880);
        // 绘制UID
        ts.color = data->config.color.profile;
        ts.font_family = syb_font; ts.font_family += syb_font_set.profile;
        ts.pointsize = SMALL_POINTSIZE;
        sprintf(stemp, "UID: %d", data->uinfo.user_id);
        image.Drawtext(stemp, ts, 585, 365);
        // 绘制QQ号
        if (data->config.qq != -1) {
            sprintf(stemp, "QQ: %ld", data->config.qq);
        } else {
            sprintf(stemp, "QQ: unknown");
        }

        image.Drawtext(stemp, ts, 585, 395);
        // 绘制国家/地区排名
        itemp = data->uinfo.country_rank - data->stat.country_rank;
        ts.color = data->config.color.profile;
        ts.font_family = syb_font; ts.font_family += syb_font_set.profile;
        ts.pointsize = SMALL_POINTSIZE;
        if (data->stat.user_id == -1) {
            sprintfS(stemp, 512, "#%d", data->uinfo.country_rank);
        } else {
            sprintfS(stemp,
                        512,
                        "#%d(%s%ld)",
                        data->uinfo.country_rank,
                        itemp > 0 ? "↓" : "↑",
                        itemp < 0 ? -itemp : itemp);
        }
        image.Drawtext(stemp, ts, 660, 460);

        // 设置Data区块字体
        ts.color = data->config.color.data;
        ts.font_family = syb_font; ts.font_family += syb_font_set.data;
        ts.pointsize = MID_POINTSIZE;
        // 绘制pp
        sprintf(stemp, "PPoint :     %.2f", data->uinfo.pp);
        image.Drawtext(stemp, ts, 140, 1210);
        // 绘制ranked score
        sprintf(stemp,
                "Ranked Score : %s",
                llToString(data->uinfo.ranked_score).c_str());
        image.Drawtext(stemp, ts, 106, 1070);
        // 绘制tth
        sprintf(stemp,
                "Total Hits :    %s",
                llToString(itemp + data->stat.total_hit).c_str());
        image.Drawtext(stemp, ts, 240, 1630);
        // 绘制pc
        sprintfS(stemp, 512, "Playcount :    %d", data->uinfo.playcount);
        image.Drawtext(stemp, ts, 173, 1350);
        // 绘制lv
        sprintf(stemp, "Current Level :   %.2f", data->uinfo.level);
        image.Drawtext(stemp, ts, 274, 1770);
        // 绘制acc
        sprintf(stemp, "Hit Accuracy : %.2f%%", data->uinfo.accuracy);
        image.Drawtext(stemp, ts, 207, 1490);
        if (data->stat.user_id != -1) {
            // 绘制pp变化
            ftemp = data->uinfo.pp - data->stat.pp;
            ts.color = ftemp < 0 ? data->config.color.arrowdown
                                    : data->config.color.arrowup;
            sprintfS(stemp,
                        512,
                        "%s%s",
                        ftemp < 0 ? "↓" : "↑",
                        DoubleRound2(ftemp < 0 ? -ftemp : ftemp).c_str());
            image.Drawtext(stemp, ts, 670, 1210);

            // 绘制ranked score变化
            itemp = data->uinfo.ranked_score - data->stat.ranked_score;
            ts.color = itemp < 0 ? data->config.color.arrowdown
                                    : data->config.color.arrowup;

            sprintfS(stemp,
                        512,
                        "%s%s",
                        itemp < 0 ? "-" : "+",
                        llToString(itemp < 0 ? -itemp : itemp).c_str());
            image.Drawtext(stemp, ts, 626, 1070);
            // 绘制tth变化
            ts.color = itemp < 0 ? data->config.color.arrowdown
                                    : data->config.color.arrowup;
            itemp = data->uinfo.n300 + data->uinfo.n100 + data->uinfo.n50
                    - data->stat.total_hit;
            sprintfS(stemp,
                        512,
                        "%s%s",
                        itemp < 0 ? "-" : "+",
                        llToString(itemp < 0 ? -itemp : itemp).c_str());
            image.Drawtext(stemp, ts, 780, 1630);

            // 绘制pc变化
            itemp = data->uinfo.playcount - data->stat.playcount;
            ts.color = itemp < 0 ? data->config.color.arrowdown
                                    : data->config.color.arrowup;
            sprintfS(stemp,
                        512,
                        "%s%ld",
                        itemp < 0 ? "-" : "+",
                        itemp < 0 ? -itemp : itemp);
            image.Drawtext(stemp, ts, 713, 1350);

            // 绘制acc变化
            dtemp = data->uinfo.accuracy - data->stat.accuracy;
            ts.color = dtemp < 0 ? data->config.color.arrowdown
                                    : data->config.color.arrowup;

            sprintf(stemp,
                    "%s%.2f%%",
                    dtemp < 0 ? "↓" : "↑",
                    dtemp < 0 ? -dtemp : dtemp);
            image.Drawtext(stemp, ts, 747, 1490);
            // 绘制lv变化
            dtemp = data->uinfo.level - data->stat.level;
            ts.color = dtemp < 0 ? data->config.color.arrowdown
                                    : data->config.color.arrowup;

            sprintf(stemp,
                    "%s%.2f",
                    dtemp < 0 ? "-" : "+",
                    dtemp < 0 ? -dtemp : dtemp);
            image.Drawtext(stemp, ts, 814, 1770);
            // 绘制rank差值
            ts.color = data->config.color.name;
            ts.font_family = syb_font; ts.font_family += syb_font_set.name;
            ts.pointsize = BIG_POINTSIZE;
            itemp = data->uinfo.count_ssh + data->uinfo.count_ss - data->stat.xh
                    - data->stat.x;
            sprintf(stemp,
                    "(%s%ld)",
                    itemp < 0 ? "↓" : "↑",
                    itemp < 0 ? 0 - itemp : itemp);
            image.Drawtext(stemp, ts, 253, 860);

            itemp = data->uinfo.count_sh + data->uinfo.count_s - data->stat.sh
                    - data->stat.s;
            sprintf(stemp,
                    "(%s%ld)",
                    itemp < 0 ? "↓" : "↑",
                    itemp < 0 ? 0 - itemp : itemp);
            image.Drawtext(stemp, ts, 494, 860);

            itemp = data->uinfo.count_a - data->stat.a;
            sprintf(stemp,
                    "(%s%ld)",
                    itemp < 0 ? "↓" : "↑",
                    itemp < 0 ? 0 - itemp : itemp);
            image.Drawtext(stemp, ts, 735, 860);
        }

        // 绘制rank数量
        const std::vector<int> count_rank = {data->uinfo.count_ss,
                                                data->uinfo.count_ssh,
                                                data->uinfo.count_s,
                                                data->uinfo.count_sh,
                                                data->uinfo.count_a};
        ts.pointsize = BIG_POINTSIZE;
        ts.color = data->config.color.name;
        for (int i = 0; i < 5; ++i) {
            image.Drawtext(std::to_string(count_rank[i]),
                            ts,
                            253 + 120 * i,
                            i % 2 ? 940 : 790);
        }
        // 绘制全球排名差值
        if (data->stat.user_id != -1) {
            itemp = data->uinfo.global_rank - data->stat.global_rank;
            sprintf(stemp,
                    "(%s%ld)",
                    itemp > 0 ? "↓" : "↑",
                    itemp < 0 ? -itemp : itemp);
            ts.color = itemp > 0 ? data->config.color.arrowdown
                                    : data->config.color.arrowup;
            ts.font_family = syb_font; ts.font_family += syb_font_set.arrow;
            image.Drawtext(stemp, ts, 660, 270);
        }

        // 绘制全球排名
        ts.color = data->config.color.name;
        ts.font_family = syb_font; ts.font_family += syb_font_set.name;
        image.Drawtext(std::to_string(data->uinfo.global_rank), ts, 600, 220);
        // 绘制名字
        image.Drawtext(data->uinfo.username, ts, 555, 325);
        // 绘制签名
        ts.color = data->config.color.sign;
        ts.font_family = syb_font; ts.font_family += syb_font_set.sign;
        ts.gravity = MagickCore::GravityType::NorthGravity;
        ts.align = MagickCore::AlignType::CenterAlign;
        image.Drawtext(data->config.sign, ts, 540, 660);
#pragma endregion
        image.Save(out_path);

        static string_t result;
        result = "[CQ:image, file=file://";
        result += out_path;
        result += "]";
        return result.c_str();
}

}