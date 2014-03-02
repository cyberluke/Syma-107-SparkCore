using System;
using System.Collections.Generic;
using System.Text;

namespace JoystickApp
{
    public class JoystickConf
    {
        public const int Min = 0;
        public const int Max = 65535;
        public const int Center = 32767;
        public const int Deadzone = 0;
        public const int XPin = 6;
        public const int YPin = 7;
        public const int ZPin = 5;
        public static Boolean Enabled = false;
    }
}
