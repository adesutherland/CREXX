/* Pure Rexx CMS-style ADDRESS environment provider */
options levelb
namespace _rxsysb expose cmsaddressenvironment
import _rxsysb

cmsaddressenvironment: class implements .addressenvironment
  _msg_mode = .string

  *: match
    arg env_name = .string
    if _address_normalize_environment_name(env_name) = "CMS" then return 100
    return 0

  *: factory
    arg env_name = .string
    _msg_mode = "ON"
    return

  execute: method = .addressresponse
    arg request = .addressrequest

    normalized = .string
    prefix8 = .string
    prefix4 = .string

    normalized = _address_normalize_command(request.get_command())
    prefix8 = _address_first_chars(normalized, 8)
    prefix4 = _address_first_chars(normalized, 4)

    if normalized = "CP SET MSG ON" then do
      _msg_mode = "ON"
      return .addressresponse(0)
    end

    if normalized = "CP SET MSG OFF" then do
      _msg_mode = "OFF"
      return .addressresponse(0)
    end

    if normalized = "CP QUERY USERID" then return _address_execute_system_command(request, "echo CMSUSER")
    if prefix8 = "LISTFILE" then return _address_execute_system_command(request, "echo DEMO EXEC A1")
    if prefix4 = "TYPE" then return _address_execute_system_command(request, "echo CMS TYPE DEMO")

    return _address_unknown_command_response("CMS", request.get_command())
