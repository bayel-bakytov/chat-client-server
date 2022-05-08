﻿using System;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;

namespace ChatClient
{
    public partial class Form1 : Form
    {
        /// <summary>
        /// The .net wrapper around WinSock sockets.
        /// </summary>
        TcpClient _client;
        /// <summary>
        /// Buffer to store incoming messages from the server.
        /// </summary>
        byte[] _buffer = new byte[4096];

        public Form1()
        {
            InitializeComponent();
            _client = new TcpClient();
        }

        public void test(string p, string l)
        {
            label2.Text = p;
            label3.Text = l;

        }

        public void auth() {
            var msg = Encoding.ASCII.GetBytes("$@" + label3.Text + "*" + label2.Text);
            _client.GetStream().Write(msg, 0, msg.Length);
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
                        if (str[0] == '@')
                        {
                            listBox2.Items.Clear();
                            string[] _str = str.Split('@');
                            foreach (string s in _str)
                            {
                                listBox2.Items.Add(s);
                            }
                        } else if (str[0] == '%') {
                            string[] _str = str.Split('@');
                            label1.Text = _str[1];
                        } else if (str[0] == '^') {
                            listBox1.Items.Clear();
                            string[] _str = str.Split('^');
                            foreach (string s in _str)
                            {
                                listBox1.Items.Add(s);
                            }
                        }
                        else
                        {
                            listBox1.Items.Add(str);
                            listBox1.SelectedIndex = listBox1.Items.Count - 1;
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

        private void button1_Click(object sender, EventArgs e)
        {
            var msg = Encoding.ASCII.GetBytes(textBox1.Text+"@"+label3.Text);
            _client.GetStream().Write(msg, 0, msg.Length);

            // Clear the text box and set it's focus
            textBox1.Text = "";
            textBox1.Focus();
        }
    }
}
