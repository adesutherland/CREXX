/* Pure Rexx CMS-style ADDRESS environment provider used by tests */
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
    response = .addressresponse
    binding = .addressbinding
    sandbox = .addresssandbox
    sandbox_value = .string

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

    if normalized = "SANDBOX ROUNDTRIP" then do
      sandbox = request.get_sandbox() as .addresssandbox
      sandbox_value = sandbox.access("GET", "VALUE.3", "")
      call sandbox.access("SET", "result", sandbox_value || ":cms")
      call sandbox.access("SET", "value.4", "cms-four")
      return .addressresponse(0)
    end

    if normalized = "SANDBOX RESPONSE UPDATE" then do
      response = .addressresponse(0)
      call response.add_updated_binding("sandbox", "response", "response", "cms-response", "")
      return response
    end

    if normalized = "SET BUFFER UPDATED" then do
      response = .addressresponse(0)
      do i = 1 to request.get_binding_count()
        binding = request.get_binding(i)
        if _address_normalize_environment_name(binding.get_kind()) = "VAR" & _address_normalize_environment_name(binding.get_internal_name()) = "BUFFER" then do
          call response.add_updated_binding("var", binding.get_internal_name(), binding.get_external_alias(), "cms-updated", "")
        end
      end
      return response
    end

    return _address_unknown_command_response("CMS", request.get_command())
