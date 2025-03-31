## Needs
 
By default Python is long path aware. We recently discovered when running our application via a service using pywin32 that our application could not access long paths even tho long path support was enabled on the host. We discovered that pythoservice.exe was not long path aware. We found that we were able to add long path support manually by using an application manifest and the Windows SDK mt.exe tool.

The expected for pywin32 is that service scripts running through pythonservice.exe are long path aware when the hosts registry has long paths enabled.

