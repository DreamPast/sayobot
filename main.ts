import os from 'os';
import path from 'path';
import superagent from 'superagent';
import fs from 'fs-extra';
import { App, getTargetId } from 'koishi-core';
import { Collection, ObjectID } from 'mongodb';
import ffi from 'ffi-napi';
import 'koishi-plugin-mongo';

const tmpdir = path.join(os.tmpdir(), 'sayobot');
fs.ensureDirSync(tmpdir);

type Args = [
    // dataColor profileColor signColor
    string, string, string,
    // mode user_id country username qq
    number, number, string, string, string,
    // sign background profileEdge dataEdge signEdge
    string, string, string, string, string,
    // opacity count300 count100 count50 playcount
    number, number, number, number, number,
    // total_score ranked_score total_hits pp_raw pp_country_rank
    number, number, number, number, number,
    // pp_rank count_ssh count_ss count_sh count_s
    number, number, number, number, number,
    // count_a total_seconds_played level accuracy stat_total_score
    number, number, number, number, number,
    // stat_ranked_score stat_total_hits stat_accuracy stat_pp_raw stat_level
    number, number, number, number, number,
    // stat_pp_rank stat_pp_country_rank stat_playcount stat_count_ssh stat_count_ss
    number, number, number, number, number,
    // stat_count_sh stat_count_s stat_count_a days out_path
    number, number, number, number, string,
];

interface Lib {
    MakePersonalCard: (...args: Args) => string,
}

const sayobot: Lib = ffi.Library(path.resolve(__dirname, 'sayobot'), {
    MakePersonalCard: ['string', [
        'string', 'string', 'string', 'int', 'int',
        'string', 'string', 'string', 'string', 'string',
        'string', 'string', 'string', 'int', 'int',
        'int', 'int', 'int', 'long long', 'long long',
        'long long', 'float', 'int', 'int', 'int',
        'int', 'int', 'int', 'int', 'int',
        'float', 'double', 'long long', 'long long', 'int',
        'double', 'float', 'float', 'int', 'int',
        'long long', 'int', 'int', 'int', 'int',
        'int', 'unsigned', 'string',
    ]]
});

const BACK_PATH = path.resolve(__dirname, 'png', 'stat');
const EDGE_PATH = path.resolve(__dirname, 'png', 'tk');
const AVATAR_PATH = path.resolve(__dirname, 'png', 'avatars');

interface GetUserResult {
    mode: number,
    username: string,
    count300: number,
    count100: number,
    count50: number,
    playcount: number,
    ranked_score: number,
    total_score: number,
    pp_rank: number,
    level: number, // double
    pp_raw: number, // double
    accuracy: number, // double
    count_rank_ss: number,
    count_rank_ssh: number,
    count_rank_s: number,
    count_rank_sh: number,
    count_rank_a: number,
    country: string,
    total_seconds_played: number,
    pp_country_rank: number,
}

interface ApiResult {
    mode: number,
    user_id: string, // int
    username: string, // string
    join_date: string, // datestr
    count300: string, // int
    count100: string, // int
    count50: string, // int
    playcount: string, // int
    ranked_score: string, // int
    total_score: string, // int
    pp_rank: string, // int
    level: string, // double
    pp_raw: string, // double
    accuracy: string, // double
    count_rank_ss: string // int
    count_rank_ssh: string // int
    count_rank_s: string // int
    count_rank_sh: string, // int
    count_rank_a: string, // int
    country: string,
    total_seconds_played: string, // int
    pp_country_rank: string // int
    events: any[],
}

interface HistoryColumn extends GetUserResult {
    _id: ObjectID, // create time
};

interface UserInfo {
    _id: number,
    account: number,
    sign: string,
    nickname: string,
    Opacity: number,
    history: HistoryColumn[],
    // relative path
    ProfileEdge: string,
    DataEdge: string,
    SignEdge: string,
    ProfileFontColor: string,
    DataFontColor: string,
    SignFontColor: string,
    Background: string,
}

namespace Api {
    export const modes = { std: 0, taiko: 1, ctb: 2, mania: 3 }

    export async function getUserIDByNick(nickname: string) {
        const result = await superagent.get(`https://api.sayobot.cn/ppy/get_user?u=${nickname}&m=0`);
        if (result.status === 404) throw new Error('阁下，小夜找不到这个人呢');
        else if (result.status !== 200) throw new Error(`Status: ${result.status}`);
        if (!result.body.length) throw new Error('阁下，小夜找不到这个人呢');
        return parseInt(result.body[0].user_id, 10);
    }

    export async function getUser(uid: number, mode: number): Promise<GetUserResult> {
        const response = await superagent.get(`https://api.sayobot.cn/ppy/get_user?u=${uid}&m=${mode}`);
        if (response.status === 404) throw new Error('阁下，小夜找不到这个人呢');
        else if (response.status !== 200) throw new Error(`Status: ${response.status}`);
        if (!response.body.length) throw new Error('阁下，小夜找不到这个人呢');
        const t: ApiResult = response.body[0];
        const result: GetUserResult = {
            mode,
            username: t.username,
            count300: parseInt(t.count300, 10),
            count100: parseInt(t.count100, 10),
            count50: parseInt(t.count50, 10),
            playcount: parseInt(t.playcount, 10),
            ranked_score: parseInt(t.ranked_score, 10),
            total_score: parseInt(t.total_score, 10),
            pp_rank: parseInt(t.pp_rank, 10),
            level: parseFloat(t.level),
            pp_raw: parseFloat(t.pp_raw),
            accuracy: parseFloat(t.accuracy),
            count_rank_ss: parseInt(t.count_rank_ss, 10),
            count_rank_ssh: parseInt(t.count_rank_ssh, 10),
            count_rank_s: parseInt(t.count_rank_s, 10),
            count_rank_sh: parseInt(t.count_rank_sh, 10),
            count_rank_a: parseInt(t.count_rank_a, 10),
            country: t.country,
            total_seconds_played: parseInt(t.total_seconds_played, 10),
            pp_country_rank: parseInt(t.pp_country_rank, 10),
        }
        return result;
    }
}

export function apply(app: App) {
    app.command('osu', 'osu').action(() => 'Use osu -h for help.');

    app.on('connect', () => {
        const coll: Collection<UserInfo> = app.database.db.collection('osu');

        function update(id: number, $set: Partial<UserInfo>) {
            return coll.updateOne({ _id: id }, { $set });
        }

        app.command('osu.set <nickname>', 'Bind osu account')
            .shortcut('！set <nickname>', { prefix: false })
            .shortcut('!set <nickname>', { prefix: false })
            .action(async ({ session }, nickname) => {
                const account = await Api.getUserIDByNick(nickname);
                await coll.updateOne(
                    { _id: session.userId },
                    { $set: { account, nickname } },
                    { upsert: true },
                );
                return '阁下绑定成功啦，发送指令！o就可以查看阁下的资料卡。还有其他指令阁下可以通过help查看哦';
            });

        app.command('osu.unset', 'Unset')
            .shortcut('！unset', { prefix: false })
            .shortcut('!unset', { prefix: false })
            .action(async ({ session }) => {
                const result = await coll.deleteOne({ _id: session.userId });
                if (!result.deletedCount) return '阁下还没绑定哦';
                return '啊咧咧，，阁下你叫什么名字呀，突然不记得了，快用 !set 告诉我吧';
            });

        app.command('osu.updateSign <sign>', 'updatesign', { checkArgCount: false })
            .shortcut('！更新框框', { prefix: false })
            .shortcut('!更新框框', { prefix: false })
            .action(async ({ session }, sign) => {
                if (!sign) return '更新个签有1个参数哦阁下';
                const userInfo = await coll.findOne({ _id: session.userId });
                if (!userInfo) return '阁下还没绑定哦，用set把阁下的名字告诉我吧';
                const res = await update(session.userId, { sign });
                return res.modifiedCount ? '更新成功' : '更新失败惹';
            });

        app.command('osu.updateEdge [arg0] [arg1]', 'updateedge', { checkArgCount: false })
            .action(async ({ session }, arg0, arg1) => {
                const userInfo = await coll.findOne({ _id: session.userId });
                if (!userInfo) return '阁下还没绑定哦，用set把阁下的名字告诉我吧';
                const fields = ['ProfileEdge', 'DataEdge', 'SignEdge'];
                const fontColors = ['ProfileFontColor', 'DataFontColor', 'SignFontColor'];
                if (!(arg0 && arg1)) return '更新框框有2个参数哦阁下';
                const index = parseInt(arg0, 10);
                if (!(index >= 0 && index <= 2)) return '更新框框第1个参数是0-2的数字哦阁下';
                const filepath = path.join(EDGE_PATH, `${arg1 + index}.png`);
                if (!fs.existsSync(filepath)) return '没有这个框框哦阁下';
                const colorFile = path.join(EDGE_PATH, `${arg1}0.col`);
                let color: string;
                if (fs.existsSync(colorFile)) {
                    color = `#${fs.readFileSync(colorFile).toString()}`;
                } else color = 'default';
                const result = await update(session.userId, { [fontColors[index]]: color, [fields[index]]: filepath });
                if (result.modifiedCount) return '更新成功';
                return '更新失败惹';
            });

        app.command('osu.updateBackground [arg0]', 'updateback', { checkArgCount: false })
            .action(async ({ session }, arg0) => {
                const userInfo = await coll.findOne({ _id: session.userId });
                if (!userInfo) return '阁下还没绑定哦，用set把阁下的名字告诉我吧';
                if (!arg0) return '更换背景有1个参数哦阁下';
                const filename = path.resolve(BACK_PATH, `${arg0}.png`);
                if (!fs.existsSync(filename)) return '没有这个背景哦阁下';
                const result = await update(session.userId, { Background: filename });
                if (result.modifiedCount) return '更新成功';
                return '更新失败惹';
            });

        app.command('osu.updateOpacity [arg0]', '更改透明度', { checkArgCount: false })
            .action(async ({ session }, arg0) => {
                const userInfo = await coll.findOne({ _id: session.userId });
                if (!userInfo) return '阁下还没绑定哦，用set把阁下的名字告诉我吧';
                if (!arg0) return '更改透明度有1个参数哦阁下';
                const opacity = parseInt(arg0, 10);
                if (Number.isNaN(opacity) || opacity % 5) return '更改透明度第1个参数是0-100以内可被5整除的数字哦阁下';
                await update(session.userId, { Opacity: opacity });
                return '更新成功';
            });

        app.command('osu.updateGravatar', '')
            .action(async ({ session }) => {
                const userInfo = await coll.findOne({ _id: session.userId });
                if (!userInfo) return '阁下还没绑定哦，用set把阁下的名字告诉我吧';
                const w = fs.createWriteStream(path.join(AVATAR_PATH, `${userInfo.account}.png`));
                superagent.get(`https://a.ppy.sh/${userInfo.account}`).pipe(w);
                return '更新头像完成，如果阁下的头像还是没有更新，等一会再试试吧';
            });

        app.command('osu.stat [userId] [day]', '', { minInterval: 3000 })
            .option('mode', '-m <mode>', { value: 'std' })
            .shortcut('！o', { prefix: false })
            .shortcut('!o', { prefix: false })
            .shortcut('！t', { prefix: false, options: { mode: 'taiko' } })
            .shortcut('!t', { prefix: false, options: { mode: 'taiko' } })
            .shortcut('！c', { prefix: false, options: { mode: 'ctb' } })
            .shortcut('!c', { prefix: false, options: { mode: 'ctb' } })
            .shortcut('！m', { prefix: false, options: { mode: 'mania' } })
            .shortcut('!m', { prefix: false, options: { mode: 'mania' } })
            .action(async ({ session, options }, userId, _day) => {
                let day = 0;
                let image = path.resolve(os.tmpdir(), new ObjectID().toHexString() + '.png');
                if (_day) {
                    day = parseInt(_day);
                    if (Number.isNaN(day)) return '天数必须是数字哦';
                }
                if (!Api.modes[options.mode]) return '未知的模式';
                let userInfo: UserInfo;
                if (userId) {
                    userInfo = await coll.findOne({ _id: getTargetId(userId) });
                    if (!userInfo) return '小夜还不认识这个人哦，阁下把他介绍给我吧';
                } else {
                    userInfo = await coll.findOne({ _id: session.userId });
                    if (!userInfo) return '阁下还没绑定哦，用set把阁下的名字告诉我吧';
                }
                if (!fs.existsSync(path.join(AVATAR_PATH, `${userInfo.account}.png`))) {
                    const result = await superagent.get(`https://a.ppy.sh/${userInfo.account}`);
                    fs.writeFileSync(path.join(AVATAR_PATH, `${userInfo.account}.png`), result.text);
                }
                if (day) {
                    let found: HistoryColumn;
                    const search = new Date().getTime() - day * 3600 * 24 * 1000;
                    for (let i = userInfo.history.length - 1; i >= 0; i--) {
                        const item = userInfo.history[i];
                        if (item._id.generationTime * 1000 < search) {
                            found = item;
                            break;
                        }
                    }
                    if (!found) return `小夜没有查到${userId ? '这个人' : '阁下'}${day}天前的信息`;
                    if (!found.playcount) return `${userId ? '这个人' : '阁下'}还没有玩过这个模式哦，赶紧去试试吧`;
                    const current = await Api.getUser(userInfo.account, Api.modes[options.mode]);
                    const currentTotal = current.count300 + current.count100 + current.count50;
                    const foundTotal = found.count300 + found.count100 + found.count50;
                    sayobot.MakePersonalCard(
                        userInfo.DataFontColor, userInfo.ProfileFontColor, userInfo.SignFontColor,
                        Api.modes[options.mode], userInfo.account, current.country, current.username, session.userId.toString(),
                        userInfo.sign, userInfo.Background, userInfo.ProfileEdge, userInfo.DataEdge, userInfo.SignEdge,
                        userInfo.Opacity, current.count300, current.count100, current.count50, current.playcount,
                        current.total_score, current.ranked_score, currentTotal, current.pp_raw, current.pp_country_rank,
                        current.pp_rank, current.count_rank_ssh, current.count_rank_ss, current.count_rank_sh, current.count_rank_s,
                        current.count_rank_a, current.total_seconds_played, current.level, current.accuracy, found.total_score,
                        found.ranked_score, foundTotal, found.accuracy, found.pp_raw, found.level,
                        found.pp_rank, found.pp_country_rank, found.playcount, found.count_rank_ssh, found.count_rank_ss,
                        found.count_rank_sh, found.count_rank_s, found.count_rank_a, day, image,
                    );
                } else {
                    const current = await Api.getUser(userInfo.account, Api.modes[options.mode]);
                    const currentTotal = current.count300 + current.count100 + current.count50;
                    await coll.updateOne({ _id: session.userId }, { $push: { history: { ...current, _id: new ObjectID() } } });
                    sayobot.MakePersonalCard(
                        userInfo.DataFontColor, userInfo.ProfileFontColor, userInfo.SignFontColor,
                        Api.modes[options.mode], userInfo.account, current.country, current.username, session.userId.toString(),
                        userInfo.sign, userInfo.Background, userInfo.ProfileEdge, userInfo.DataEdge, userInfo.SignEdge,
                        userInfo.Opacity, current.count300, current.count100, current.count50, current.playcount,
                        current.total_score, current.ranked_score, currentTotal, current.pp_raw, current.pp_country_rank,
                        current.pp_rank, current.count_rank_ssh, current.count_rank_ss, current.count_rank_sh, current.count_rank_s,
                        current.count_rank_a, current.total_seconds_played, current.level, current.accuracy, current.total_score,
                        current.ranked_score, currentTotal, current.accuracy, current.pp_raw, current.level,
                        current.pp_rank, current.pp_country_rank, current.playcount, current.count_rank_ssh, current.count_rank_ss,
                        current.count_rank_sh, current.count_rank_s, current.count_rank_a, day, image,
                    );
                }
                return `[CQ:image,file=base64://${fs.readFileSync(image).toString('base64')}]`;
            });

        app.command('osu.help [category]', 'Get help')
            .action(async (_, category) => {
                if (!category) return `[CQ:image,file=file://${path.resolve(__dirname, 'png', 'help', 'Sayobot help.png')}]`;
                if (!['框框', '背景', '个签', '大括号'].includes(category)) return '没有这个帮助的图片哦';
                return `[CQ:image,file=file://${path.resolve(__dirname, 'png', 'help', `help-${category}.png`)}]`;
            });

        app.command('osu.download', '?', { hidden: true })
            .shortcut('好无聊啊', { prefix: false })
            .action(() => "Welcome to OSU! 点击下载osu客户端 https://txy1.sayobot.cn/osu.zip 点击在线游玩 http://game.osu.sh");

        app.on('message', async (session) => {
            if (session.message.startsWith('https://osu.ppy.sh/')) {
                return session.$send(`点击链接下载此图 https://osu.sayobot.cn/?search=${session.message.split('beatmapsets/')[1]}`);
            }
        });
    });
}
