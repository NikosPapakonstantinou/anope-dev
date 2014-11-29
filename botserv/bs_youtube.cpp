        const string YouTubeLinkRegex = @"http(?:s?)://(?:www\.)?youtu(?:be\.com/watch\?v=|\.be/)([\w\-]+)(&(amp;)?[\w\?=‌​]*)?";
        const string YouTubeGdataApiPrefix = "https://gdata.youtube.com/feeds/api/videos/";

        string IPlugin.InvokeWithMessage(string source, string message, ref IrcClient client)
        {
            string to_send = "";
            MatchCollection matches = Regex.Matches(message, YouTubeLinkRegex);
            foreach (Match m in matches)
            {
                XmlDocument xd = new XmlDocument();
                try
                {
                    xd.Load(YouTubeGdataApiPrefix + m.Groups[1].Value);
                }
                catch (WebException e)
                {
                    Debug.WriteLine(e.ToString(), "TitlePlugin");
                    return null; // we failed
                }
                XmlNamespaceManager nsmgr = new XmlNamespaceManager(xd.NameTable);
                nsmgr.AddNamespace("atom", "http://www.w3.org/2005/Atom");
                nsmgr.AddNamespace("media", "http://search.yahoo.com/mrss/");
                nsmgr.AddNamespace("yt", "http://gdata.youtube.com/schemas/2007");
                string author = xd.SelectSingleNode("//atom:author/atom:name", nsmgr).InnerText;
                string views = xd.SelectSingleNode("//yt:statistics", nsmgr).Attributes["viewCount"].InnerText;
                DateTime published = DateTime.Parse(xd.SelectSingleNode("//atom:published", nsmgr).InnerText);
                TimeSpan ts = new TimeSpan(0, 0, Convert.ToInt32(xd.SelectSingleNode("//media:group/yt:duration", nsmgr).Attributes["seconds"].InnerText));
                to_send = String.Format("YouTube video posted by {0} on {1}, with {2} views ({3} long)", author, published.ToShortDateString(), views, ts);
			}
        }			
				
				

				
		public class Title : IPlugin
    {
        const string UrlMatchExpression = @"(?i)\b((?:https?://|www\d{0,3}[.]|[a-z0-9.\-]+[.][a-z]{2,4}/)(?:[^\s()<>]+|\(([^\s()<>]+|(\([^\s()<>]+\)))*\))+(?:\(([^\s()<>]+|(\([^\s()<>]+\)))*\)|[^\s`!()\[\]{};:'" + "\"" + ".,<>?«»“”‘’]))";

        string IPlugin.InvokeWithMessage(string source, string message, ref IrcClient client)
        {
            string toSend = String.Empty; // Make csc happy
            // catch urls
            MatchCollection matches = Regex.Matches(message, UrlMatchExpression);
            foreach (Match m in matches)
            {
                if (!(m.Value.StartsWith("http://") || m.Value.StartsWith("https://"))) continue; // boo unprefix

                try
                {
                    // Check if that's even an HTML file
                    WebRequest wr = WebRequest.Create(m.Value);
                    wr.Method = "HEAD";
                    string type = String.Empty;
                    using (WebResponse wrr = wr.GetResponse())
                    {
                        Debug.WriteLine("Found type " + wrr.ContentType, "TitlePlugin");
                        type = wrr.ContentType;
                    }
                    // TODO: Support a whole bunch of wacky shit. img2aa anyone?
                    // Go through the types. We StartWith because of encoding info.
                    if (type.StartsWith("text/html"))
                    {
                        // We could support the other wacky shit like XML
                        toSend = GetHTMLGist(m.Value);
                    }
                }
                catch (WebException e)
                {
                    Debug.WriteLine(e.ToString(), "TitlePlugin");
                    return null; // we failed
                }
                catch (UriFormatException e)
                {
                    Debug.WriteLine(e.ToString(), "TitlePlugin");
                    return null; // we failed
                }
            }
            return !toSend.IsNullOrWhitespace() ? toSend : null;
        }

        string IPlugin.InvokeWithChannelUserChange(string channel, string user, string kicker, string message, ChannelUserChange type, ref IrcClient client)
        {
            return null; // Not implemented
        }

        /// <summary>
        /// Gets the title and any other interesting doodads of an HTML document.
        /// </summary>
        /// <param name="url"></param>
        /// <returns>The title, maybe other stuff..</returns>
        static string GetHTMLGist(string url)
        {
            HtmlDocument hd = new HtmlDocument();
            using (WebClient wc = new WebClient())
            {
                hd.LoadHtml(wc.DownloadString(url));
            }
            try
            {
                string title = hd.DocumentNode.SelectSingleNode("//title").InnerText;
                Debug.WriteLine("Title is " + title, "TitlePlugin");
                return title;
            }
            catch (NullReferenceException e)
            {
                Debug.WriteLine(e.ToString(), "TitlePlugin");
                return null;
            }
        }
    }		
