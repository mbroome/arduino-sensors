// list of nodes
#define KITCHEN1_NODE_ID     1
#define SALTWATER1_NODE_ID   2
#define LIVINGROOM1_NODE_ID  3
#define FISHROOM1_NODE_ID    4
#define AMPCLAMP1_NODE_ID    5
#define BASEMENT1_NODE_ID    6
#define BASEMENT2_NODE_ID    7
#define BASEMENT3_NODE_ID    8

bool offsetDiff(float a, float b, float offset){
   Serial.print("a: ");
   Serial.print(a);
   Serial.print(" b: ");
   Serial.print(b);
   Serial.print(" diff: ");
   float x = abs(a - b);
   Serial.println(x);
   if(x > offset){
      return(true);
   }else{
      return(false);
   }
}