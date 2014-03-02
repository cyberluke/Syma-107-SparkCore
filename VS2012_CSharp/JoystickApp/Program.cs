// Copyright (c) 2010-2013 SharpDX - Alexandre Mutel
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

using System;
using SharpDX.DirectInput;
using JoystickApp;
using System.Threading;

namespace EnumDevicesApp
{
    static class Program
    {
        private static TcpServer Server;
        private static S107ServiceProvider Provider;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        //[STAThread]
        static void Main()
        {
            Provider = new S107ServiceProvider();
            Server = new TcpServer(Provider, 9000);
            Server.Start();
            MainForJoystick();
            //MainForKeyboard();
        }

        static void MainForJoystick()
        {
            // Initialize DirectInput
            var directInput = new DirectInput();

            // Find a Joystick Guid
            var joystickGuid = Guid.Empty;

            foreach (var deviceInstance in directInput.GetDevices(DeviceType.Gamepad, DeviceEnumerationFlags.AllDevices))
                joystickGuid = deviceInstance.InstanceGuid;

            // If Gamepad not found, look for a Joystick
            if (joystickGuid == Guid.Empty)
                foreach (var deviceInstance in directInput.GetDevices(DeviceType.Joystick, DeviceEnumerationFlags.AllDevices))
                    joystickGuid = deviceInstance.InstanceGuid;

            // If Joystick not found, throws an error
            if (joystickGuid == Guid.Empty)
            {
                Console.WriteLine("No joystick/Gamepad found.");
                Console.ReadKey();
                Environment.Exit(1);
            }

            // Instantiate the joystick
            var joystick = new Joystick(directInput, joystickGuid);

            Console.WriteLine("Found Joystick/Gamepad with GUID: {0}", joystickGuid);

            // Query all suported ForceFeedback effects
            var allEffects = joystick.GetEffects();
            foreach (var effectInfo in allEffects)
                Console.WriteLine("Effect available {0}", effectInfo.Name);

            // Set BufferSize in order to use buffered data.
            joystick.Properties.BufferSize = 128;

            // Acquire the joystick
            joystick.Acquire();

            // Poll events from joystick

            int axisCoef = 512;
            byte[] message = new byte[3];
            byte XPin = 63;
            byte YPin = 63;
            byte ZPin = 0;
            message[0] = XPin;
            message[1] = YPin;
            message[2] = ZPin;

            while (true)
            {
                joystick.Poll();
                var datas = joystick.GetBufferedData();
                //if (Server.CurrentConnections > 0)
                //{
                    foreach (var state in datas)
                    {
                        //Console.WriteLine(state);
                        if (state.Offset.Equals(JoystickOffset.X))
                        {
                            if (state.Value >= JoystickConf.Min && state.Value < JoystickConf.Center - JoystickConf.Deadzone)
                            {
                                // LEFT
                                XPin = (byte)((JoystickConf.Max - state.Value) / axisCoef);
                            }
                            else if (state.Value > JoystickConf.Center + JoystickConf.Deadzone)
                            {
                                // RIGHT
                                XPin = (byte)((JoystickConf.Max - state.Value) / axisCoef);
                            }
                        }
                        else if (state.Offset.Equals(JoystickOffset.Y))
                        {
                            if (state.Value >= JoystickConf.Min && state.Value < JoystickConf.Center - JoystickConf.Deadzone)
                            {
                                // UP
                                YPin = (byte)(state.Value / axisCoef);
                            }
                            else if (state.Value > JoystickConf.Center + JoystickConf.Deadzone)
                            {
                                // DOWN
                                YPin = (byte)(state.Value / axisCoef);
                            }

                        }
                        else if (state.Offset.Equals(JoystickOffset.Z))
                        {
                            // THROTTLE
                            ZPin = Convert.ToByte((JoystickConf.Max - state.Value) / axisCoef);
                        }
                    }
                    if (message[0] != XPin || message[1] != YPin || message[2] != ZPin || ZPin == 127)
                    {
                        if (ZPin > 115)
                        {
                            ZPin = 127;
                        }
                        if (ZPin == 1)
                        {
                            ZPin = 0;
                        }
                        if (XPin == 64 || XPin == 65)
                        {
                            XPin = 63;
                        }
                        if (YPin == 64 || YPin == 65)
                        {
                            YPin = 63;
                        }
                        message[0] = XPin;
                        message[1] = YPin;
                        message[2] = ZPin;
                        Server.SendToAll(message);
                        //Thread.Sleep(60);
                        Console.WriteLine(message[0] + " " + message[1] + " " + message[2]);
                    }
                }
                
            //}
        }


        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        //[STAThread]
        static void MainForKeyboard()
        {
            // Initialize DirectInput
            var directInput = new DirectInput();

            // Instantiate the joystick
            var keyboard = new Keyboard(directInput);

            // Acquire the joystick
            keyboard.Properties.BufferSize = 128;
            keyboard.Acquire();

            // Poll events from joystick
            while (true)
            {
                keyboard.Poll();
                var datas = keyboard.GetBufferedData();
                foreach (var state in datas)
                    Console.WriteLine(state);
            }
        }

    }
}
