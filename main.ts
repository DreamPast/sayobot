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

const BACK_PATH = path.resolve(__dirname, 'png', 'stat');
const EDGE_PATH = path.resolve(__dirname, 'png', 'tk');

interface HistoryColumn {
    _id: ObjectID, // create time
    username: string,
    total_score: number,
    ranked_score: number,
    total_hit: number,
    accuracy: number,
    level: number,
    pp: number,
    global_rank: number,
    country_rank: number,
    country: string,
    playcount: number,
    xh: number,
    x: number,
    sh: number,
    s: number,
    a: number,
    mode: number,
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
    export enum mode { std, taiko, ctb, mania }

    export async function getUserIDByNick(nickname: string) {
        const result = await superagent.get(`https://api.sayobot.cn/ppy/get_user?u=${nickname}&m=0`);
        if (result.status === 404) throw new Error('阁下，小夜找不到这个人呢');
        else if (result.status !== 200) throw new Error(`Status: ${result.status}`);
        if (!result.body.length) throw new Error('阁下，小夜找不到这个人呢');
        return parseInt(result.body[0].user_id, 10);
    }

    export async function getUser(uid: number) {

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
            .action(async ({ session }) => {
                const result = await coll.deleteOne({ _id: session.userId });
                if (!result.deletedCount) return '阁下还没绑定哦';
                return '啊咧咧，，阁下你叫什么名字呀，突然不记得了，快用 !set 告诉我吧';
            });

        app.command('osu.updateSign <sign>', 'updatesign', { checkArgCount: false })
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
                const w = fs.createWriteStream(path.join(tmpdir, `${userInfo.account}.png`));
                superagent.get(`https://a.ppy.sh/${userInfo.account}`).pipe(w);
                return '更新头像完成，如果阁下的头像还是没有更新，等一会再试试吧';
            });

        app.command('osu.stat [userId] [day]', '')
            .shortcut('！o', { prefix: false })
            .shortcut('!o', { prefix: false })
            .action(async ({ session }, userId, _day) => {
                let day = 0;
                if (_day) {
                    day = parseInt(_day);
                    if (Number.isNaN(day)) return '天数必须是数字哦';
                }
                let userInfo: UserInfo;
                if (userId) {
                    userInfo = await coll.findOne({ _id: getTargetId(userId) });
                    if (!userInfo) return '小夜还不认识这个人哦，阁下把他介绍给我吧';
                } else {
                    userInfo = await coll.findOne({ _id: session.userId });
                    if (!userInfo) return '阁下还没绑定哦，用set把阁下的名字告诉我吧';
                }
                if (!fs.existsSync(path.join(tmpdir, `${userInfo.account}.png`))) {
                    const result = await superagent.get(`https://a.ppy.sh/${userInfo.account}`);
                    fs.writeFileSync(path.join(tmpdir, `${userInfo.account}.png`), result.text);
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
                    return await sayobot.draw(found);
                } else {
                    // getUser
                    const found = Api.getUser(userInfo.account);
                    return await sayobot.draw(found);
                }
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
