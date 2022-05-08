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

namespace ChatClient
{
    public partial class SignUp : Form
    {

        /// <summary>
        /// The .net wrapper around WinSock sockets.
        /// </summary>
        TcpClient _client;
        SignIn signForm = new SignIn();
        /// <summary>
        /// Buffer to store incoming messages from the server.
        /// </summary>
        byte[] _buffer = new byte[4096];

        public SignUp()
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

        }



        private void button1_Click(object sender, EventArgs e)
        {

            var msg = Encoding.ASCII.GetBytes("/@" + textBox1.Text + "*" + textBox2.Text);
            _client.GetStream().Write(msg, 0, msg.Length);

            signForm.Show();
            this.Visible = false;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            signForm.Show();
            this.Visible = false;
        }
    }
}
