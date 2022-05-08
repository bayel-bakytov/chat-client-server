using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using ChatClient;

namespace ChatClient
{
    public partial class SignIn : Form
    {
        /// <summary>
        /// The .net wrapper around WinSock sockets.
        /// </summary>
        TcpClient _client;
        Form1 form1 = new Form1();

        public static string login;
        public static string password;
        /// <summary>
        /// Buffer to store incoming messages from the server.
        /// </summary>
        byte[] _buffer = new byte[4096];

        public SignIn()
        {
            InitializeComponent();
            _client = new TcpClient();
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);

            // Connect to the remote server. The IP address and port # could be
            // picked up from a settings file.
            _client.Connect("26.253.250.211", 54000);

            // Start reading the socket and receive any incoming messages
            _client.GetStream().BeginRead(_buffer,
                                            0,
                                            _buffer.Length,
                                            Server_MessageReceived,
                                            null);
        }

        private void Server_MessageReceived(IAsyncResult ar)
        {
            if (ar.IsCompleted)
            {
                // End the stream read
                var bytesIn = _client.GetStream().EndRead(ar);
                if (bytesIn > 0)
                {
                    // Create a string from the received data. For this server 
                    // our data is in the form of a simple string, but it could be
                    // binary data or a JSON object. Payload is your choice.
                    var tmp = new byte[bytesIn];
                    Array.Copy(_buffer, 0, tmp, 0, bytesIn);
                    var str = Encoding.ASCII.GetString(tmp);

                    // Any actions that involve interacting with the UI must be done
                    // on the main thread. This method is being called on a worker
                    // thread so using the form's BeginInvoke() method is vital to
                    // ensure that the action is performed on the main thread.
                    BeginInvoke((Action)(() =>
                    {
                        if (str[0] == '+')
                        {
                            string[] _str = str.Split('@','*');
                            form1.test(_str[2], _str[1]);
                            
                            form1.Show();
                            
                            //this.Visible = false;
                        }
                        else
                        {
                            label4.Text = "Неверный логин или пароль!";
                        }
                        
                    }));
                }
                
                // Clear the buffer and start listening again
                Array.Clear(_buffer, 0, _buffer.Length);
                _client.GetStream().BeginRead(_buffer,
                                                0,
                                                _buffer.Length,
                                                Server_MessageReceived,
                                                null);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            SignUp f = new SignUp();
            f.Show();
            this.Visible = false;
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            var msg = Encoding.ASCII.GetBytes("\\@" + textBox1.Text + "*" + textBox2.Text);
            _client.GetStream().Write(msg, 0, msg.Length);
        }
    }
}

