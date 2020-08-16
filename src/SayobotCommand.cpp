#ifndef WIN32
    #define sprintf_s(buffer, length, format, ...) sprintf(buffer, format, __VA_ARGS__)
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

#include "SayobotException.hpp"
#include "SayobotHttp.hpp"
#include "SayobotImage.hpp"
#include "osu_api.hpp"

#pragma region path
    #define BACKGROUND_PATH "../png/stat/"
    #define EDGE_PATH "../png/tk/"
    #define FONT_PATH "../fonts/"
    #define SKIN_PATH "../png/rank/"
    #define COUNTRY_PATH "../png/country/"
    #define GLOBAL_PATH "../png/world/s.png"
    #define AVATAR_PATH "../png/avatars/"
#pragma endregion

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

string_t Saybot_llToString(int64_t n) {
    const char digit[4] = {' ', 'K', 'M', 'G'};
    long double v = n;
    int a = 0;
    while (v > 1000.0) {
        v /= 1000.0;
        ++a;
    }
    char *buf = new char[20];
    sprintf_s(buf, 20, "%.2Lf", v);
    string_t ret(buf);
    delete[] buf;
    return ret + digit[a];
}

string_t Saybot_DoubleRound2(double a) {
    char *buf = new char[20];
    sprintf_s(buf, 20, "%.2lf", a);
    string_t ret(buf);
    delete[] buf;
    return ret;
}

extern "C" {

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
    osu_api::mode mode;
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

enum mode_enum { std = 0, taiko, ctb, mania };

SAYOBOT_API struct UserPanelData {
    mode_enum mode;
    user_info uinfo;
    UserConfigData config;
    UserStatData stat;
    int compareDays;
};

SAYOBOT_API char* MakePersonalCard(const UserPanelData* data, const char* path) {
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
            sprintf(stemp, BACKGROUND_PATH "%s", data->config.background);
            image.DrawPic(stemp, 0, 0);
            // 不透明贴图
            sprintf(stemp, "../png/fx%d.png", data->config.opacity);
            image.DrawPic(stemp, 0, 0);
            // 绘制个人信息框
            sprintf(stemp, EDGE_PATH "%s", data->config.edge.profile);
            image.DrawPic(stemp, 50, 20, 970, 600);
            // 绘制数据框
            for (int i = 0; i < 6; ++i) {
                sprintf(stemp, EDGE_PATH "%s", data->config.edge.data);
                image.DrawPic(stemp, 56 + 33.5 * i, 980 + 140 * i, 820, 140);
            }

            // 绘制签名框
            sprintf(stemp, EDGE_PATH "%s", data->config.edge.sign);
            image.DrawPic(stemp, 125, 570, 825, 150);
            // 绘制头像
            sprintf_s(stemp, 512, AVATAR_PATH "%d.png", data->uinfo.user_id);
            try {
                image.DrawPic(stemp, 165, 150, 350, 350);
            } catch (Magick::Exception &ex) {
                image.DrawPic(AVATAR_PATH "no-avatar.png", 165, 150, 350, 350);
            }
            // 绘制模式图标
            sprintf(stemp,
                    SKIN_PATH "%s%s",
                    data->config.skin,
                    mode_str[(int)data->mode]);
            image.DrawPic(stemp, 165, 150, 80, 80);
            // 绘制地球图标
            image.DrawPic(GLOBAL_PATH, 510, 150, 100, 100);
            // 绘制国旗
            sprintf(stemp,
                    COUNTRY_PATH "%s.png",
                    (data->uinfo.country && *data->uinfo.country) ? data->uinfo.country : "__");
            image.DrawPic(stemp, 560, 425, 80, 80);

            // 绘制rank图标
            for (int i = 0; i < 5; ++i) {
                sprintf(stemp,
                        SKIN_PATH "%s%s",
                        data->config.skin,
                        rank_str[i]);
                image.DrawPic(stemp, 165 + 120 * i, i % 2 ? 870 : 720, 82, 98);
            }
            Sayobot::TextStyle ts;
            // 绘制天数
            ts.color = data->config.color.time;
            ts.font_family = FONT_PATH + data->config.font.time;
            ts.pointsize = SMALL_POINTSIZE;
            if (data->compareDays != 0) {
                sprintf_s(stemp, 512, "compare with %u days ago", data->compareDays);
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
            ts.font_family = FONT_PATH + data->config.font.profile;
            ts.pointsize = SMALL_POINTSIZE;
            sprintf(stemp, "UID: %d", data->uinfo.user_id);
            image.Drawtext(stemp, ts, 585, 365);
            // 绘制QQ号
            if (data->config.qq != -1) {
                sprintf(stemp, "QQ: %lld", data->config.qq);
            } else {
                sprintf(stemp, "QQ: unknown");
            }

            image.Drawtext(stemp, ts, 585, 395);
            // 绘制国家/地区排名
            itemp = data->uinfo.country_rank - data->stat.country_rank;
            ts.color = data->config.color.profile;
            ts.font_family = FONT_PATH + data->config.font.profile;
            ts.pointsize = SMALL_POINTSIZE;
            if (data->stat.user_id == -1) {
                sprintf_s(stemp, 512, "#%d", data->uinfo.country_rank);
            } else {
                sprintf_s(stemp,
                          512,
                          "#%d(%s%lld)",
                          data->uinfo.country_rank,
                          itemp > 0 ? "↓" : "↑",
                          itemp < 0 ? -itemp : itemp);
            }
            image.Drawtext(stemp, ts, 660, 460);

            // 设置Data区块字体
            ts.color = data->config.color.data;
            ts.font_family = FONT_PATH + data->config.font.data;
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
            sprintf_s(stemp, 512, "Playcount :    %d", data->uinfo.playcount);
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
                sprintf_s(stemp,
                          512,
                          "%s%s",
                          ftemp < 0 ? "↓" : "↑",
                          DoubleRound2(ftemp < 0 ? -ftemp : ftemp).c_str());
                image.Drawtext(stemp, ts, 670, 1210);

                // 绘制ranked score变化
                itemp = data->uinfo.ranked_score - data->stat.ranked_score;
                ts.color = itemp < 0 ? data->config.color.arrowdown
                                     : data->config.color.arrowup;

                sprintf_s(stemp,
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
                sprintf_s(stemp,
                          512,
                          "%s%s",
                          itemp < 0 ? "-" : "+",
                          llToString(itemp < 0 ? -itemp : itemp).c_str());
                image.Drawtext(stemp, ts, 780, 1630);

                // 绘制pc变化
                itemp = data->uinfo.playcount - data->stat.playcount;
                ts.color = itemp < 0 ? data->config.color.arrowdown
                                     : data->config.color.arrowup;
                sprintf_s(stemp,
                          512,
                          "%s%lld",
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
                ts.font_family = FONT_PATH + data->config.font.name;
                ts.pointsize = BIG_POINTSIZE;
                itemp = data->uinfo.count_ssh + data->uinfo.count_ss - data->stat.xh
                        - data->stat.x;
                sprintf(stemp,
                        "(%s%lld)",
                        itemp < 0 ? "↓" : "↑",
                        itemp < 0 ? 0 - itemp : itemp);
                image.Drawtext(stemp, ts, 253, 860);

                itemp = data->uinfo.count_sh + data->uinfo.count_s - data->stat.sh
                        - data->stat.s;
                sprintf(stemp,
                        "(%s%lld)",
                        itemp < 0 ? "↓" : "↑",
                        itemp < 0 ? 0 - itemp : itemp);
                image.Drawtext(stemp, ts, 494, 860);

                itemp = data->uinfo.count_a - data->stat.a;
                sprintf(stemp,
                        "(%s%lld)",
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
                        "(%s%lld)",
                        itemp > 0 ? "↓" : "↑",
                        itemp < 0 ? -itemp : itemp);
                ts.color = itemp > 0 ? data->config.color.arrowdown
                                     : data->config.color.arrowup;
                ts.font_family = FONT_PATH + data->config.font.arrow;
                image.Drawtext(stemp, ts, 660, 270);
            }

            // 绘制全球排名
            ts.color = data->config.color.name;
            ts.font_family = FONT_PATH + data->config.font.name;
            image.Drawtext(std::to_string(data->uinfo.global_rank), ts, 600, 220);
            // 绘制名字
            image.Drawtext(data->uinfo.username, ts, 555, 325);
            // 绘制签名
            ts.color = data->config.color.sign;
            ts.font_family = FONT_PATH + data->config.font.sign;
            ts.gravity = MagickCore::GravityType::NorthGravity;
            ts.align = MagickCore::AlignType::CenterAlign;
            image.Drawtext(data->config.sign, ts, 540, 660);
#pragma endregion
            string_t hash = image.GetRandomHash();
            string_t filename = "data/image/" + hash + ".jpg";
            image.Save(filename);
            return "[CQ:image, file=file://" + outputImagePath + "]";
}
