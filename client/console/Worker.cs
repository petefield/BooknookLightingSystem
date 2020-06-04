using System;
using System.Collections;
using System.Net.Http;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Hosting;

namespace console
{
    public class Worker : BackgroundService
    {
        private readonly IHttpClientFactory _httpClientFactory;

        public Worker(IHttpClientFactory httpClientFactory)
        {
            _httpClientFactory = httpClientFactory;
        }

        protected override async Task ExecuteAsync(CancellationToken stoppingToken)
        {
            var httpClient = _httpClientFactory.CreateClient();

            while (!stoppingToken.IsCancellationRequested)
            {
                foreach (var charByte in Encoding.ASCII.GetBytes("I love You"))
                {
                    var bits = new BitArray(new[] { charByte });

                    for (var led = 0; led < 8; led++)
                    {
                        var value = bits[led] ? "1023" : "0000";
                        httpClient.PostAsync($"http://booknook.local/{led}/{value}", null);
                        Console.Write($"{value} ");
                    }
                    Console.WriteLine();
                    await Task.Delay(500, stoppingToken);
                }

                await Task.Delay(5000, stoppingToken);
                Console.WriteLine();
                Console.WriteLine();
            }
        }
    }
}
