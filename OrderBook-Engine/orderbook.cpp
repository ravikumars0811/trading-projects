#include <iostream>
#include <map>
#include <queue>
#include <string>
using namespace std;

struct Order { int id; string side; double price; int qty; };

map<double, queue<Order>, greater<double>> bids;
map<double, queue<Order>, less<double>> asks;

void matchOrder(Order o) {
    if(o.side=="BUY"){
        while(o.qty>0 && !asks.empty() && asks.begin()->first<=o.price){
            auto &q=asks.begin()->second;
            Order sell=q.front(); q.pop();
            int traded=min(o.qty,sell.qty);
            o.qty-=traded; sell.qty-=traded;
            cout<<"Trade: BUY "<<traded<<" @ "<<sell.price<<endl;
            if(sell.qty>0) q.push(sell);
            if(q.empty()) asks.erase(asks.begin());
        }
        if(o.qty>0) bids[o.price].push(o);
    } else {
        while(o.qty>0 && !bids.empty() && bids.begin()->first>=o.price){
            auto &q=bids.begin()->second;
            Order buy=q.front(); q.pop();
            int traded=min(o.qty,buy.qty);
            o.qty-=traded; buy.qty-=traded;
            cout<<"Trade: SELL "<<traded<<" @ "<<buy.price<<endl;
            if(buy.qty>0) q.push(buy);
            if(q.empty()) bids.erase(bids.begin());
        }
        if(o.qty>0) asks[o.price].push(o);
    }
}

int main(){
    matchOrder({1,"BUY",100.0,10});
    matchOrder({2,"SELL",99.5,5});
    matchOrder({3,"SELL",100.0,10});
}
