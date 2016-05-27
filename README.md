# Veritrans IOS Library

### We are in the process of depricating this SDK, and will be publishing our latest SDK Soon. Please contact support@veritrans.co.id for more information

### Description
By using this library, you will be able to securely collect credit card information without exposing sensitive credit card information to your servers. This library includes:
  * Get Token : getting token (includes 3D Secure transaction) from Veritrans payment gateway by using [VT-Direct](http://docs.veritrans.co.id/en/vtdirect/integration.html).
  * Register Card : register user's credit card information to Veritrans in exchange of saved_token_id for the next transaction.

### INSTALLATION
There are seven steps for your apps to include our iOS library.

1. Create a folder named `lib` under your root project folder (this is optional, you can always name your folder for something else).
2. Copy all files inside `VTiOSAPI folder` and `include folder` from our release to the `lib folder`.
3. Drag the `lib folder` into your XCode Project Structure.
4. Click on the Project Structure and choose Build Settings tab.
5. Search for Objective-C Bridging Header under Swift Compiler - Code Generation.
6. Double click on input field and put $SOURCE_ROOT/lib/include/VTiOSAPI-BridgingHeader.h (make sure include folder is inside your root project)
7. Now you can use VTiOSAPI in your project.

Note: All files is available in our latest release.

## Get Token

### Flow Process
![VT-Direct Mobile Flow](http://docs.veritrans.co.id/images/vtdirect-mobile-flow.png)

There are three participants during a payment process in VT-Direct: Merchant, Customer, and Veritrans. Here's the explanation of how VT-Direct operates:

1. Click Pay: Customer will fill the credit card information on the iOS apps, and click Pay to complete payment.

2. Send Card Data: When the customer click the pay button, iOS library will combine the sensitive credit card information with the client key. The combination of the data is then sent to Veritrans server to be exchanged with the onetime-token ("Token").

3. Get Token: Veritrans will return the Token to the iOS apps. The token will be valid for 3 minutes during a standard credit card transaction and 10 minutes if a 3D Secure feature is utilized. Token will expire after exceeding the respective time.

4. Send Token: Token is then sent to the merchant server. Do note that the merchant will only receive Token as a substitution for the credit card details. Merchant will not save or process any credit card information. Therefore, merchant does not have to encounter any PCI DSS security issue.

5. Request Charge: Merchant will perform charge transaction with the Token, with addition of a few other information (Example: Customer details, product details, address, etc).

6. Transaction Response Handling: Veritrans returns transaction status response, such as:
   * Capture
   * Settlement


7. Send transaction status: At last, merchant will send the transaction result to customer.

### Retrieving Token

1 Set your VTConfig for Sandbox Mode and Client Key
```objective-c
//set environment
VTConfig.setVT_IsProduction(false)

//set client key
VTConfig.setCLIENT_KEY("< Your Client Key >");
```

2 Create VTDirect Object and set card details to get your token for current transaction
```objective-c
//Create VTDirect Object. Its config is Based on VTConfig Class that you set previously
var vtDirect = VTDirect()

//Create VTCardDetails Object to be set to VTDirect Object
var cardDetails = VTCardDetails()

//TODO: Set your card details based on user input.
//this is a sample
cardDetails.card_number = "4811111111111114"
cardDetails.card_cvv = "123"
cardDetails.card_exp_month = 1
cardDetails.card_exp_year = 2020;

//set true or false to enable or disable 3dsecure
cardDetails.secure = true
cardDetails.gross_amount = "100000"

//Set VTCardDetails to VTDirect
vtDirect.card_details = cardDetails
```

3 Request Token
```objective-c
//Simply Call getToken function and put your callback to handle data
vtDirect.getToken { (responseData : NSData!, ex : NSException!) -> Void in
    if (ex == nil){
        //checking response data nil or not
        if (responseData != nil) {
            var requestData = JSON(data: responseData); //parse responseData to JSON
            if requestData["redirect_url"] != nil {
                //Displaying redirect url in Web View - 3DS
                let webView:UIWebView = UIWebView(frame: CGRectMake(0, 10, 320, 320))
                webView.loadRequest(NSURLRequest(URL: NSURL(string: jsonData["redirect_url"].stringValue)!))
                webView.delegate = self
                webView.scalesPageToFit = true;
                webView.multipleTouchEnabled = false;
                webView.contentMode = UIViewContentMode.ScaleAspectFit;
                self.view.addSubview(webView)
            }
            else {
                //charge transaction - non 3DS
                self.chargeRequest("token-id=\(requestData["token_id"].stringValue)") //refer to chargeRequest function on step 5
            }
        } else {
            print("Unable to retrieve token")
        }
    } else {
        //Something is wrong, get details message by print ex.reason
        print(ex.reason)
    }
}
```

4 Create a callback function handler for 3D Secure web view
```objective-c
func webViewDidFinishLoad(webView: UIWebView) {
    // charge transaction after user has successfully authenticated by 3D Secure
    if(webView.request?.URL!.absoluteString.rangeOfString("callback") != nil){
        webView.removeFromSuperview()
        let requestData = "token-id=\(self.jsonData["token_id"].stringValue)&price=\(totalPrice)"
        chargeRequest(requestData) //refer to chargeRequest function on step 5
    }
}
```

5 Send token to your server
After getting your token to your callback, you should send it to your server for example for charging.
```objective-c
func chargeRequest(requestData: String) {
    var request = NSMutableURLRequest(URL: NSURL(string: "<PUT_YOUR_CHARGING_ENPOINT_HERE>")!)
    var session = NSURLSession.sharedSession()

    // set HTTP request config
    request.HTTPMethod = "POST"
    request.HTTPBody = requestData.dataUsingEncoding(NSUTF8StringEncoding)
    request.addValue("application/json", forHTTPHeaderField: "Accept")

    //Asynchronously get data from your server
    var task = session.dataTaskWithRequest(request, completionHandler: { (data, response, error) -> Void in
        if(error == nil){
            //we’ve got response from server, print it here
            var strData = NSString(data: data, encoding: NSUTF8StringEncoding)
            print("Body: \(strData!)")

        }else{
            //something is wrong, print localizedDescription
            print("Error: \(error.localizedDescription)")
        }
    })
    task.resume()
}
```

6 After sending your token to your server, now it’s time to charge your customer using your server key. In this example we will use PHP as our processing server with [Veritrans PHP Library](https://github.com/veritrans/veritrans-php).

File : index.php
```php
<?php
  include_once("Veritrans.php");

  if(empty($_POST['token-id'])){
      echo "Empty token-id";
      exit;
  }

  Veritrans_Config::$serverKey = "< Your server key >";
  Veritrans_Config::$isProduction = false;

  $token_id = $_POST['token-id'];

  $transaction_details = array(
      'order_id'    => rand(),
      'gross_amount'  => $_POST['price']
  );

  $transaction_data = array(
      'payment_type'      => 'credit_card',
      'credit_card'       => array(
        'token_id'  => $token_id,
        'bank'    => 'bni'
        ),
      'transaction_details'   => $transaction_details,
  );
  try{
      $result = Veritrans_VtDirect::charge($transaction_data);
      $json = json_encode($result);
      echo "{\"status\":\"success\", \"body\" : $json}";
  }catch(Exception $e){
      echo "{\"status\":\"error\",\"body\": \".$e.\"}";
  }
?>
```
If you are using another programming language, you can checkout another library that already available [here](http://docs.veritrans.co.id/en/welcome/pluginlibrary.html).

For a working example of an iOS application using this library you can check [here](https://github.com/dannypranoto/vt-iOSlib-SampleStore).

## Register Card
1 Set your VTConfig for Sandbox Mode and Client Key
```objective-c
//set environment
VTConfig.setVT_IsProduction(false)

//set client key
VTConfig.setCLIENT_KEY("< Your Client Key >");
```

2 Create VTDirect Object and set card details to get your token for current transaction
```objective-c
//Create VTDirect Object. Its config is Based on VTConfig Class that you set previously
var vtDirect = VTDirect()

//Create VTCardDetails Object to be set to VTDirect Object
var cardDetails = VTCardDetails()

//TODO: Set your card details based on user input.
//this is a sample
cardDetails.card_number = "4811111111111114"
cardDetails.card_cvv = "123"
cardDetails.card_exp_month = 1
cardDetails.card_exp_year = 2020;

//set true or false to enable or disable 3dsecure
cardDetails.secure = true
cardDetails.gross_amount = "100000"

//Set VTCardDetails to VTDirect
vtDirect.card_details = cardDetails
```

3 Register user's credit card information to your server
```objective-c
// simply call registerCard function and put your callback to handle response data
vtDirect.registerCard{(responseData: NSData!, ex: NSException!) -> Void in
    if(ex == nil) {
        // checking response data nil or not
        if (responseData != nil) {
            // here you will receive our response data from Veritrans which includes status_code, saved_token_id, transaction_id and masked_card
            var requestBody = JSON(data:responseData!)
            requestBody["user_id"] = "<PUT_YOUR_USER_ID>" // put your unique user_id to associate user's credit card ownership

            var request = NSMutableURLRequest(URL: NSURL(string: "<PUT_YOUR_REGISTER_CARD_ENPOINT_HERE>/creditcard")!)
            var session = NSURLSession.sharedSession()

            // set HTTP request config
            request.HTTPMethod = "POST"
            request.HTTPBody = requestData.dataUsingEncoding(NSUTF8StringEncoding)
            request.addValue("application/json", forHTTPHeaderField: "Accept")

            let task = session.dataTaskWithRequest(request, completionHandler: {(data, response, error) -> Void in
                if (error == nil) {
                    // if your server has successfully retrieve the saved_token_id
                    print("Success saving user's saved_token_id")
                } else {
                    print("Failed saving user's saved_token_id")
                }
            })
            task.resume()
        } else {
            print("Unable to parse")
        }
    } else {
        print(ex.reason);
    }
}
```

4 Using the saved_token_id for the next transaction.
```objective-c
//Create VTDirect Object. Its config is Based on VTConfig Class that you set previously
var vtDirect = VTDirect()

//Create VTCardDetails Object to be set to VTDirect Object
var cardDetails = VTCardDetails()

// Instead of using credit card information, just use the saved_token_id and set two_click attribute to true
cardDetails.token_id = "<PUT_YOUR_SAVED_TOKEN_ID_HERE>"
cardDetails.card_cvv = "123"
cardDetails.two_click = true

//set true or false to enable or disable 3dsecure
cardDetails.secure = true
cardDetails.gross_amount = "100000"

//Set VTCardDetails to VTDirect
vtDirect.card_details = cardDetails
```
The SDK will then send the following GET request to MERCHANT_SERVER_ADDRESS/creditcard :
```json
{
  "masked_card": "xxxxxx-xxxx",
  "saved_token_id": "xxxxxxxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
  "status_code": "xxx",
  "transaction_id": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
  "user_id": "xxxxxxx"
}
```

5 For the next steps which are getting token and charging transaction is just the same with step 3,4,5 in Get Token section.
