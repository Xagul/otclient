local currentRsa = OTSERV_RSA

function g_game.getRsa()
  return currentRsa
end

function g_game.chooseRsa(host)
  if host:match('.*\.tibia\.com') or host:match('.*\.cipsoft\.com') then
    currentRsa = CIPSOFT_RSA
  else
    currentRsa = OTSERV_RSA
  end
end

function g_game.setRsa(rsa)
  currentRsa = rsa
end

function g_game.isOfficialTibia()
  return currentRsa == CIPSOFT_RSA
end

function g_game.getOsType()
  if g_game.isOfficialTibia() then
    if g_app.getOs() == 'windows' then
      return OsTypes.Windows
    else
      return OsTypes.Linux
    end
  else
    if g_app.getOs() == 'windows' then
      return OsTypes.OtclientWindows
    elseif g_app.getOs() == 'mac' then
      return OsTypes.OtclientMac
    else
      return OsTypes.OtclientLinux
    end
  end
end

function g_game.getSupportedProtocols()
  return {
    810, 853, 854, 860, 861, 862, 870, 940,
    953, 954, 960
  }
end

