// Twitter Bot using the twit node package
// https://github.com/ttezel/twit

console.log("The illuminati are waiting for you. Reach out to feel the light...");

// import the twit npm module / package
var Twit = require('twit');
// import node file system
var fs = require("fs");

// create twit object with given keys and tokens
var T = new Twit({
	consumer_key:  		 'bcXQaqyUyQOwpHOY85fTU4dF6'
  , consumer_secret: 	 'CVhNq2DJaf5Vdys7Dmf5g14c3W5R1ydj8osNqiBYosoHmLgyah'
  , access_token: 		 '4849938233-FYIG6JvMptBt8FNbv39KvPcKKiq7YKY46FWhJar'
  , access_token_secret: 'IcJzlr453M9ZLgS9jCdUlNR0zMmIREVnH328ETQr20OKZ'
})

// process that can run independently
var exec = require('child_process').exec;

// people who follow this bot get an image in return

// set up user stream
var stream = T.stream('user');

// any time someone follows me, call the callback function followed
stream.on('follow', followed);
// could also be 'tweet'

function followed(eventMsg) {

	// find the screenName and save to file
	var name = eventMsg.source.name;
	var screenName = eventMsg.source.screen_name;

	var fileName = 'data/newluminati.txt'

	fs.appendFile(fileName, screenName, fileMade);

	function fileMade(err, data, response) {
		console.log("Successfully saved to file the new member, " + screenName);

		// generate the image and post the tweet
		// terminal command line that launches image generator

		var cmd = "open twitter_bot_image_gen_2Debug.app/";

		// execute command and call callback function
		exec(cmd, getTimeOut);

		function getTimeOut() {
			setTimeout(uploadMedia, 1000);
		}

		// tweet the image now that it's done loading
		function uploadMedia() {

			console.log('Openframeworks image processing completed. Image saved to file');

			var imgPath = 'data/newluminati.jpg'
			var imgParams = {
				encoding: 'base64'
			}
			var b64content = fs.readFileSync(imgPath, imgParams)

			// first we must post the media to Twitter
			var uploadParams = {
				media_data: b64content
			}

			T.post('media/upload', uploadParams, postTweet)

			function postTweet(err, data, response) {

				// now we can reference the media and post a tweet 
				// (media will attach to the tweet)
				var mediaIdStr = data.media_id_string
			  	var postParams = { 
			  		status: 'Welcome to the Illuminaten, @' + screenName, 
			  		media_ids: [mediaIdStr] 
			  	}

			  	T.post('statuses/update', postParams, gotData);

			  	function gotData(err, data, response) {
					if (err) {
						console.log("**** ERROR BEGIN ****");
						console.log(err);
						console.log("**** ERROR END ******");
					} else {
						console.log("Tweet successfully posted:");
						console.log("**time** " + (data.created_at).substr(0, 19));
						console.log("**text** " + data.text);
					}
				}
			}
		}
	}
}