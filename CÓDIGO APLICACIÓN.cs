using Android.App;
using Android.OS;
using Android.Runtime;
using Android.Bluetooth;
using Android.Widget;
using System;
using System.IO;
using System.Text;
using AndroidX.AppCompat.App;
using Java.Util;
using System.Threading.Tasks;

namespace App1
{
    [Activity(Label = "PROYECTO PE3", Theme = "@style/AppTheme", MainLauncher = true)]
    public class MainActivity : AppCompatActivity
    {
        // Declara las variables globales
        private BluetoothAdapter bluetoothAdapter; // El adaptador Bluetooth del dispositivo
        private BluetoothDevice bluetoothDevice; // El dispositivo Bluetooth al que se quiere conectar
        private BluetoothSocket bluetoothSocket; // El socket Bluetooth para la comunicación
        private Stream inStream; // El flujo de entrada para leer datos desde el dispositivo Bluetooth
        private Stream outStream; // El flujo de salida para escribir datos al dispositivo Bluetooth
        private Button connectButton; // El botón para conectar o desconectar el dispositivo Bluetooth
        private Button sendI; // El botón para enviar un mensaje al dispositivo Bluetooth
        private Button sendD;
        private Button sendA;
        private Button sendB;
        private Button sendC;
        private TextView statusText; // El campo de texto para mostrar el estado de la conexión
        private TextView dataText;
        string valor;
        protected override void OnCreate(Bundle savedInstanceState)
        {
            base.OnCreate(savedInstanceState);
            Xamarin.Essentials.Platform.Init(this, savedInstanceState);
            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.activity_main);
            // Obtiene los elementos de la interfaz de usuario
            connectButton = FindViewById<Button>(Resource.Id.connectButton);
            sendI = FindViewById<Button>(Resource.Id.sendI);
            sendD = FindViewById<Button>(Resource.Id.sendD);
            sendA = FindViewById<Button>(Resource.Id.sendA);
            sendB = FindViewById<Button>(Resource.Id.sendB);
            sendC = FindViewById<Button>(Resource.Id.sendC);
            statusText = FindViewById<TextView>(Resource.Id.statusText);
            dataText = FindViewById<TextView>(Resource.Id.dataText);
            
            // Obtiene una instancia del adaptador Bluetooth del dispositivo
            bluetoothAdapter = BluetoothAdapter.DefaultAdapter;
            // Comprueba si el dispositivo tiene Bluetooth y si está activado
            if (bluetoothAdapter == null)
            {
                Toast.MakeText(this, "Este dispositivo no tiene Bluetooth", ToastLength.Long).Show();
                return;
            }
            if (!bluetoothAdapter.IsEnabled)
            {
                Toast.MakeText(this, "Por favor, activa el Bluetooth", ToastLength.Long).Show();
                return;
            }
            // Asigna los eventos a los botones
            connectButton.Click += (s, e) => {     // Comprueba si el socket Bluetooth está conectado o no
                if (bluetoothSocket == null || !bluetoothSocket.IsConnected)
                {
                    // Intenta conectar el socket Bluetooth al dispositivo HC-05
                    try
                    {
                        // Obtiene el dispositivo Bluetooth con el nombre "HC-05"
                        bluetoothDevice = bluetoothAdapter.GetRemoteDevice("00:22:12:01:86:43");

                        // Crea un socket Bluetooth con el UUID estándar para SPP (Serial Port Profile)
                        bluetoothSocket = bluetoothDevice.CreateRfcommSocketToServiceRecord(UUID.FromString("00001101-0000-1000-8000-00805F9B34FB"));
                        // Configura la velocidad de baudios
                       
                        // Conecta el socket Bluetooth al dispositivo HC-05
                        bluetoothSocket.Connect();
                        
                        // Obtiene los flujos de entrada y salida del socket Bluetooth
                        inStream = bluetoothSocket.InputStream;
                        outStream = bluetoothSocket.OutputStream;

                        // Cambia el estado de la conexión a conectado y el texto del botón a desconectar
                        statusText.Text = "Conectado";
                        connectButton.Text = "Desconectar";

                        // Muestra un mensaje de éxito en la pantalla
                        Toast.MakeText(this, "Conexión establecida con éxito", ToastLength.Short).Show();
                    }
                    catch (Exception ex)
                    {
                        // Muestra un mensaje de error en la pantalla
                        Toast.MakeText(this, "Error al conectar: " + ex.Message, ToastLength.Long).Show();
                    }
                }
                else
                {
                    // Intenta desconectar el socket Bluetooth del dispositivo HC-05
                    try
                    {
                        // Cierra los flujos de entrada y salida del socket Bluetooth
                        inStream.Close();
                        outStream.Close();

                        // Cierra el socket Bluetooth
                        bluetoothSocket.Close();

                        // Cambia el estado de la conexión a desconectado y el texto del botón a conectar
                        statusText.Text = "Desconectado";
                        connectButton.Text = "Conectar";

                        // Muestra un mensaje de éxito en la pantalla
                        Toast.MakeText(this, "Conexión cerrada con éxito", ToastLength.Short).Show();
                    }
                    catch (Exception ex)
                    {
                        // Muestra un mensaje de error en la pantalla
                        Toast.MakeText(this, "Error al desconectar: " + ex.Message, ToastLength.Long).Show();
                    }
                }
            };
            sendI.Click += (s, e) => {senmsg("I");
                if (sendI.Text== "INICIAR") { dataText.Text = valor ; sendI.Text = "PAUSAR";  } 
                else if (sendI.Text=="PAUSAR") { dataText.Text = valor;  sendI.Text = "INICIAR"; }
                beginListenForData();
            };
            sendD.Click += (s, e) => {senmsg("D"); dataText.Text = "00"; valor = "00"; sendI.Text = "INICIAR"; };
            sendA.Click += (s, e) => {senmsg("A");};
            sendB.Click += (s, e) => {senmsg("B");};
            sendC.Click += (s, e) => {senmsg("C");};
            // Inicializa el estado de la conexión como desconectado
            statusText.Text = "Desconectado";

        }

        private void senmsg(string caharsend)
        {

            // Comprueba si el socket Bluetooth está conectado o no
            if (bluetoothSocket == null || !bluetoothSocket.IsConnected)
            {
                // Muestra un mensaje de advertencia en la pantalla
                Toast.MakeText(this, "Por favor, conecta el dispositivo Bluetooth primero", ToastLength.Long).Show();
                return;
            }
            // Comprueba si el mensaje está vacío o no
            if (string.IsNullOrEmpty(caharsend))
            {
                // Muestra un mensaje de advertencia en la pantalla
                Toast.MakeText(this, "Por favor, introduce un mensaje a enviar", ToastLength.Long).Show();
                return;
            }
            // Intenta enviar el mensaje al dispositivo Bluetooth
            try
            {
                // Convierte el mensaje a un array de bytes
                byte[] buffer = Encoding.ASCII.GetBytes(caharsend);
                // Escribe el array de bytes en el flujo de salida del socket Bluetooth
                outStream.Write(buffer, 0, buffer.Length);
                // Muestra un mensaje de éxito en la pantalla
                Toast.MakeText(this, "Mensaje enviado con éxito", ToastLength.Short).Show();
            }
            catch (Exception ex)
            {
                // Muestra un mensaje de error en la pantalla
                Toast.MakeText(this, "Error al enviar el mensaje: " + ex.Message, ToastLength.Long).Show();
            }
           
        }

        public override void OnRequestPermissionsResult(int requestCode, string[] permissions, [GeneratedEnum] Android.Content.PM.Permission[] grantResults)
        {
            Xamarin.Essentials.Platform.OnRequestPermissionsResult(requestCode, permissions, grantResults);

            base.OnRequestPermissionsResult(requestCode, permissions, grantResults);
        }
       
        //Evento para inicializar el hilo que escuchara las peticiones del bluetooth
        public void beginListenForData()
        {
            //Extraemos el stream de entrada
            try
            {
                inStream = bluetoothSocket.InputStream;
            }
            catch (System.IO.IOException ex)
            {
                Console.WriteLine(ex.Message);
            }
            //Creamos un hilo que estara corriendo en background el cual verificara si hay algun dato
            //por parte del arduino
            Task.Factory.StartNew(() => {
                //declaramos el buffer donde guardaremos la lectura
                byte[] buffer = new byte[1024];
                //declaramos el numero de bytes recibidos
                int bytes;
                while (true)
                {
                    try
                    {
                        //leemos el buffer de entrada y asignamos la cantidad de bytes entrantes
                        bytes = inStream.Read(buffer, 0, buffer.Length);
                        //Verificamos que los bytes contengan informacion
                        if (bytes > 0)
                        {
                            //Corremos en la interfaz principal
                            RunOnUiThread(() => {
                                //Convertimos el valor de la informacion llegada a string
                                 valor = System.Text.Encoding.ASCII.GetString(buffer);
                                //Agregamos a nuestro label la informacion llegada
                                dataText.Text =   valor;
                            });
                        }
                    }
                    catch (Java.IO.IOException)
                    {
                        //En caso de error limpiamos nuestra label y cortamos el hilo de comunicacion
                        RunOnUiThread(() => {
                            dataText.Text = string.Empty;
                        });
                        break;
                    }
                }
            });
        }

    }
}


