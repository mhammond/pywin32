## Needs
 
By default Python is long path aware. We recently discovered when running our application via a service using pywin32 that our application could not access long paths even tho long path support was enabled on the host. We discovered that pythoservice.exe was not long path aware. We should not be modifying the executable after it has been build. The docs ApplicationManifest.md, mt-exe.md and ApplicationManifest.md show how Windows works and are helper files that pywin32 can use as refernce. 

The expected for pywin32 is that service scripts running through pythonservice.exe are long path aware when the hosts registry has long paths enabled.

