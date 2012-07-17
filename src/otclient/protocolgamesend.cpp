/*
 * Copyright (c) 2010-2012 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "protocolgame.h"
#include "game.h"

void ProtocolGame::safeSend(const OutputMessagePtr& outputMessage)
{
    // avoid usage of automated sends (bot modules)
    if(!g_game.checkBotProtection())
        return;
    Protocol::send(outputMessage);
}

void ProtocolGame::sendExtendedOpcode(uint8 opcode, const std::string& buffer)
{
    if(m_enableSendExtendedOpcode) {
        OutputMessagePtr msg(new OutputMessage);
        msg->addU8(Proto::ClientExtendedOpcode);
        msg->addU8(opcode);
        msg->addString(buffer);
        safeSend(msg);
    } else {
        g_logger.error(stdext::format("Unable to send extended opcode %d, extended opcodes are not enabled", opcode));
    }
}

void ProtocolGame::sendLoginPacket(uint challangeTimestamp, uint8 challangeRandom)
{
    OutputMessagePtr msg(new OutputMessage);

    msg->addU8(Proto::ClientEnterGame);

    msg->addU16(g_lua.callGlobalField<int>("g_game", "getOs"));
    msg->addU16(g_game.getProtocolVersion());

    int paddingBytes = 128;
    msg->addU8(0); // first RSA byte must be 0
    paddingBytes -= 1;

    // xtea key
    generateXteaKey();
    msg->addU32(m_xteaKey[0]);
    msg->addU32(m_xteaKey[1]);
    msg->addU32(m_xteaKey[2]);
    msg->addU32(m_xteaKey[3]);
    msg->addU8(0); // is gm set?
    paddingBytes -= 17;

    if(g_game.getFeature(Otc::GameProtocolChecksum))
        enableChecksum();

    if(g_game.getFeature(Otc::GameAccountNames)) {
        msg->addString(m_accountName);
        msg->addString(m_characterName);
        msg->addString(m_accountPassword);
        paddingBytes -= 6 + m_accountName.length() + m_characterName.length() + m_accountPassword.length();
    } else {
        msg->addU32(stdext::from_string<uint32>(m_accountName));
        msg->addString(m_characterName);
        msg->addString(m_accountPassword);
        paddingBytes -= 8 + m_characterName.length() + m_accountPassword.length();
    }

    if(g_game.getFeature(Otc::GameChallangeOnLogin)) {
        msg->addU32(challangeTimestamp);
        msg->addU8(challangeRandom);
        paddingBytes -= 5;
    }

    // complete the 128 bytes for rsa encryption with zeros
    msg->addPaddingBytes(paddingBytes);

    // encrypt with RSA
    msg->encryptRsa(128, g_lua.callGlobalField<std::string>("g_game", "getRsa"));

    send(msg);

    enableXteaEncryption();
}

void ProtocolGame::sendLogout()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientLeaveGame);
    send(msg);
}

void ProtocolGame::sendPing()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientPing);
    send(msg);
}

void ProtocolGame::sendPingBack()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientPingBack);
    send(msg);
}

void ProtocolGame::sendAutoWalk(const std::vector<Otc::Direction>& path)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientAutoWalk);
    msg->addU8(path.size());
    for(Otc::Direction dir : path) {
        uint8 byte;
        switch(dir) {
            case Otc::East:
                byte = 1;
                break;
            case Otc::NorthEast:
                byte = 2;
                break;
            case Otc::North:
                byte = 3;
                break;
            case Otc::NorthWest:
                byte = 4;
                break;
            case Otc::West:
                byte = 5;
                break;
            case Otc::SouthWest:
                byte = 6;
                break;
            case Otc::South:
                byte = 7;
                break;
            case Otc::SouthEast:
                byte = 8;
                break;
            default:
                byte = 0;
                break;
        }
        msg->addU8(byte);
    }
    send(msg);
}

void ProtocolGame::sendWalkNorth()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientWalkNorth);
    send(msg);
}

void ProtocolGame::sendWalkEast()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientWalkEast);
    send(msg);
}

void ProtocolGame::sendWalkSouth()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientWalkSouth);
    send(msg);
}

void ProtocolGame::sendWalkWest()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientWalkWest);
    send(msg);
}

void ProtocolGame::sendStop()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientStop);
    send(msg);
}

void ProtocolGame::sendWalkNorthEast()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientWalkNorthEast);
    send(msg);
}

void ProtocolGame::sendWalkSouthEast()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientWalkSouthEast);
    send(msg);
}

void ProtocolGame::sendWalkSouthWest()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientWalkSouthWest);
    send(msg);
}

void ProtocolGame::sendWalkNorthWest()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientWalkNorthWest);
    send(msg);
}

void ProtocolGame::sendTurnNorth()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientTurnNorth);
    send(msg);
}

void ProtocolGame::sendTurnEast()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientTurnEast);
    send(msg);
}

void ProtocolGame::sendTurnSouth()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientTurnSouth);
    send(msg);
}

void ProtocolGame::sendTurnWest()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientTurnWest);
    send(msg);
}

void ProtocolGame::sendEquipItem(int itemId, int countOrSubType)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientEquipItem);
    msg->addU16(itemId);
    msg->addU8(countOrSubType);
    send(msg);
}

void ProtocolGame::sendMove(const Position& fromPos, int thingId, int stackpos, const Position& toPos, int count)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientMove);
    addPosition(msg, fromPos);
    msg->addU16(thingId);
    msg->addU8(stackpos);
    addPosition(msg, toPos);
    msg->addU8(count);
    send(msg);
}

void ProtocolGame::sendInspectNpcTrade(int itemId, int count)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientInspectNpcTrade);
    msg->addU16(itemId);
    msg->addU8(count);
    send(msg);
}

void ProtocolGame::sendBuyItem(int itemId, int subType, int amount, bool ignoreCapacity, bool buyWithBackpack)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientBuyItem);
    msg->addU16(itemId);
    msg->addU8(subType);
    msg->addU8(amount);
    msg->addU8(ignoreCapacity ? 0x01 : 0x00);
    msg->addU8(buyWithBackpack ? 0x01 : 0x00);
    send(msg);
}

void ProtocolGame::sendSellItem(int itemId, int subType, int amount, bool ignoreEquipped)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientSellItem);
    msg->addU16(itemId);
    msg->addU8(subType);
    msg->addU8(amount);
    msg->addU8(ignoreEquipped ? 0x01 : 0x00);
    send(msg);
}

void ProtocolGame::sendCloseNpcTrade()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientCloseNpcTrade);
    send(msg);
}

void ProtocolGame::sendRequestTrade(const Position& pos, int thingId, int stackpos, uint creatureId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRequestTrade);
    addPosition(msg, pos);
    msg->addU16(thingId);
    msg->addU8(stackpos);
    msg->addU32(creatureId);
    send(msg);
}

void ProtocolGame::sendInspectTrade(bool counterOffer, int index)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientInspectTrade);
    msg->addU8(counterOffer ? 0x01 : 0x00);
    msg->addU8(index);
    send(msg);
}

void ProtocolGame::sendAcceptTrade()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientAcceptTrade);
    send(msg);
}

void ProtocolGame::sendRejectTrade()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRejectTrade);
    send(msg);
}

void ProtocolGame::sendUseItem(const Position& position, int itemId, int stackpos, int index)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientUseItem);
    addPosition(msg, position);
    msg->addU16(itemId);
    msg->addU8(stackpos);
    msg->addU8(index);
    send(msg);
}

void ProtocolGame::sendUseItemWith(const Position& fromPos, int itemId, int fromStackpos, const Position& toPos, int toThingId, int toStackpos)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientUseItemWith);
    addPosition(msg, fromPos);
    msg->addU16(itemId);
    msg->addU8(fromStackpos);
    addPosition(msg, toPos);
    msg->addU16(toThingId);
    msg->addU8(toStackpos);
    send(msg);
}

void ProtocolGame::sendUseOnCreature(const Position& pos, int thingId, int stackpos, uint creatureId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientUseOnCreature);
    addPosition(msg, pos);
    msg->addU16(thingId);
    msg->addU8(stackpos);
    msg->addU32(creatureId);
    send(msg);
}

void ProtocolGame::sendRotateItem(const Position& pos, int thingId, int stackpos)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRotateItem);
    addPosition(msg, pos);
    msg->addU16(thingId);
    msg->addU8(stackpos);
    send(msg);
}

void ProtocolGame::sendCloseContainer(int containerId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientCloseContainer);
    msg->addU8(containerId);
    send(msg);
}

void ProtocolGame::sendUpContainer(int containerId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientUpContainer);
    msg->addU8(containerId);
    send(msg);
}

void ProtocolGame::sendEditText(uint id, const std::string& text)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientEditText);
    msg->addU32(id);
    msg->addString(text);
    send(msg);
}

void ProtocolGame::sendEditList(uint id, int doorId, const std::string& text)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientEditList);
    msg->addU8(doorId);
    msg->addU32(id);
    msg->addString(text);
    send(msg);
}

void ProtocolGame::sendLook(const Position& position, int thingId, int stackpos)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientLook);
    addPosition(msg, position);
    msg->addU16(thingId);
    msg->addU8(stackpos);
    send(msg);
}

void ProtocolGame::sendTalk(Otc::SpeakType speakType, int channelId, const std::string& receiver, const std::string& message)
{
    if(message.length() > 255 || message.length() <= 0)
        return;

    int serverSpeakType = Proto::translateSpeakTypeToServer(speakType);

    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientTalk);
    msg->addU8(serverSpeakType);

    switch(serverSpeakType) {
    case Proto::ServerSpeakPrivateFrom:
    case Proto::ServerSpeakPrivateRedFrom:
        msg->addString(receiver);
        break;
    case Proto::ServerSpeakChannelYellow:
    case Proto::ServerSpeakChannelRed:
        msg->addU16(channelId);
        break;
    }

    msg->addString(message);
    send(msg);
}

void ProtocolGame::sendRequestChannels()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRequestChannels);
    send(msg);
}

void ProtocolGame::sendJoinChannel(int channelId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientJoinChannel);
    msg->addU16(channelId);
    send(msg);
}

void ProtocolGame::sendLeaveChannel(int channelId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientLeaveChannel);
    msg->addU16(channelId);
    send(msg);
}

void ProtocolGame::sendOpenPrivateChannel(const std::string& receiver)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientOpenPrivateChannel);
    msg->addString(receiver);
    send(msg);
}

void ProtocolGame::sendCloseNpcChannel()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientCloseNpcChannel);
    send(msg);
}

void ProtocolGame::sendChangeFightModes(Otc::FightModes fightMode, Otc::ChaseModes chaseMode, bool safeFight)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientChangeFightModes);
    msg->addU8(fightMode);
    msg->addU8(chaseMode);
    msg->addU8(safeFight ? 0x01: 0x00);
    send(msg);
}

void ProtocolGame::sendAttack(uint creatureId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientAttack);
    msg->addU32(creatureId);
    msg->addU32(0);
    msg->addU32(0);
    send(msg);
}

void ProtocolGame::sendFollow(uint creatureId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientFollow);
    msg->addU32(creatureId);
    send(msg);
}

void ProtocolGame::sendInviteToParty(uint creatureId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientInviteToParty);
    msg->addU32(creatureId);
    send(msg);
}

void ProtocolGame::sendJoinParty(uint creatureId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientJoinParty);
    msg->addU32(creatureId);
    send(msg);
}

void ProtocolGame::sendRevokeInvitation(uint creatureId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRevokeInvitation);
    msg->addU32(creatureId);
    send(msg);
}

void ProtocolGame::sendPassLeadership(uint creatureId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientPassLeadership);
    msg->addU32(creatureId);
    send(msg);
}

void ProtocolGame::sendLeaveParty()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientLeaveParty);
    send(msg);
}

void ProtocolGame::sendShareExperience(bool active, int unknown)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientShareExperience);
    msg->addU8(active ? 0x01 : 0x00);

    if(g_game.getProtocolVersion() < 910)
        msg->addU8(unknown);

    send(msg);
}

void ProtocolGame::sendOpenOwnChannel()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientOpenOwnChannel);
    send(msg);
}

void ProtocolGame::sendInviteToOwnChannel(const std::string& name)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientInviteToOwnChannel);
    msg->addString(name);
    send(msg);
}

void ProtocolGame::sendExcludeFromOwnChannel(const std::string& name)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientExcludeFromOwnChannel);
    msg->addString(name);
    send(msg);
}

void ProtocolGame::sendCancelAttackAndFollow()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientCancelAttackAndFollow);
    send(msg);
}

void ProtocolGame::sendRefreshContainer()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRefreshContainer);
    send(msg);
}

void ProtocolGame::sendRequestOutfit()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRequestOutfit);
    send(msg);
}

void ProtocolGame::sendChangeOutfit(const Outfit& outfit)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientChangeOutfit);
    msg->addU16(outfit.getId());
    msg->addU8(outfit.getHead());
    msg->addU8(outfit.getBody());
    msg->addU8(outfit.getLegs());
    msg->addU8(outfit.getFeet());
    msg->addU8(outfit.getAddons());
    if(g_game.getFeature(Otc::GamePlayerMounts))
        msg->addU16(outfit.getMount());
    send(msg);
}

void ProtocolGame::sendMountStatus(bool mount)
{
    if(g_game.getFeature(Otc::GamePlayerMounts)) {
        OutputMessagePtr msg(new OutputMessage);
        msg->addU8(Proto::ClientMount);
        msg->addU8(mount);
        send(msg);
    } else {
        g_logger.error("ProtocolGame::sendMountStatus does not support the current protocol.");
    }
}

void ProtocolGame::sendAddVip(const std::string& name)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientAddVip);
    msg->addString(name);
    send(msg);
}

void ProtocolGame::sendRemoveVip(uint playerId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRemoveVip);
    msg->addU32(playerId);
    send(msg);
}

void ProtocolGame::sendBugReport(const std::string& comment)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientBugReport);
    msg->addString(comment);
    send(msg);
}

void ProtocolGame::sendRuleVilation(const std::string& target, int reason, int action, const std::string& comment, const std::string& statement, int statementId, bool ipBanishment)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRuleViolation);
    msg->addString(target);
    msg->addU8(reason);
    msg->addU8(action);
    msg->addString(comment);
    msg->addString(statement);
    msg->addU16(statementId);
    msg->addU8(ipBanishment);
    send(msg);
}

void ProtocolGame::sendDebugReport(const std::string& a, const std::string& b, const std::string& c, const std::string& d)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientDebugReport);
    msg->addString(a);
    msg->addString(b);
    msg->addString(c);
    msg->addString(d);
    send(msg);
}

void ProtocolGame::sendRequestQuestLog()
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRequestQuestLog);
    send(msg);
}

void ProtocolGame::sendRequestQuestLine(int questId)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRequestQuestLine);
    msg->addU16(questId);
    send(msg);
}

void ProtocolGame::sendNewNewRuleViolation(int reason, int action, const std::string& characterName, const std::string& comment, const std::string& translation)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientNewRuleViolation);
    msg->addU8(reason);
    msg->addU8(action);
    msg->addString(characterName);
    msg->addString(comment);
    msg->addString(translation);
    send(msg);
}

void ProtocolGame::sendRequestItemInfo(int itemId, int index)
{
    OutputMessagePtr msg(new OutputMessage);
    msg->addU8(Proto::ClientRequestItemInfo);
    msg->addU8(1); // count, 1 for just one item
    msg->addU16(itemId);
    msg->addU8(index);
    send(msg);
}

void ProtocolGame::addPosition(const OutputMessagePtr& msg, const Position& position)
{
    msg->addU16(position.x);
    msg->addU16(position.y);
    msg->addU8(position.z);
}
